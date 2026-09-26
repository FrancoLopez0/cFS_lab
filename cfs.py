#!/usr/bin/env python3
"""Create and build simple cFS applications in this mission bundle."""

import argparse
import re
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
NAME_PATTERN = re.compile(r"[a-z][a-z0-9_]*\Z")
CPUS = ("cpu1", "cpu2")


def validate_name(name):
    if not NAME_PATTERN.fullmatch(name) or len(name) > 20:
        raise ValueError("App name must be 1-20 lowercase letters, digits or underscores, starting with a letter")


def create_app(root, name, cpu="cpu1"):
    validate_name(name)
    if cpu not in CPUS:
        raise ValueError(f"Unknown CPU: {cpu}")

    app_dir = root / "apps" / name
    targets_file = root / "sample_defs" / "targets.cmake"
    startup_file = root / "sample_defs" / "generate_startup.cmake"
    targets = targets_file.read_text()
    startup = startup_file.read_text()

    module_roots = ("apps", "libs", "cfe/modules", "psp/fsw/modules")
    if any((root / base / name).exists() for base in module_roots):
        raise ValueError(f"A module named {name} already exists")
    if not re.search(rf"(?im)^SET\({cpu}_APPLIST\b", targets):
        raise ValueError(f"{cpu}_APPLIST is missing from {targets_file}")
    if "    # the rest of the apps can vary by config" not in startup:
        raise ValueError(f"Startup generator has an unexpected layout: {startup_file}")
    if re.search(rf"(?m)^\s*list\(APPEND {cpu}_APPLIST {re.escape(name)}\)", targets):
        raise ValueError(f"App is already registered: {name}")

    symbol = name.upper()
    app_dir.joinpath("fsw", "src").mkdir(parents=True)
    app_dir.joinpath("CMakeLists.txt").write_text(
        f"project(CFS_{symbol} C)\n\n"
        f"add_cfe_app({name} fsw/src/{name}_app.c)\n"
    )
    app_dir.joinpath("fsw", "src", f"{name}_app.c").write_text(
        '#include "cfe.h"\n\n'
        f"void {symbol}_AppMain(void)\n"
        "{\n"
        "    uint32 RunStatus = CFE_ES_RunStatus_APP_RUN;\n\n"
        f'    CFE_ES_WriteToSysLog("{symbol}: starting\\n");\n\n'
        "    while (CFE_ES_RunLoop(&RunStatus))\n"
        "    {\n"
        "        /* Add periodic work or message handling here. */\n"
        "        OS_TaskDelay(1000);\n"
        "    }\n\n"
        "    CFE_ES_ExitApp(RunStatus);\n"
        "}\n"
    )

    # These generated apps use the standard cFE API, so exclude EDS builds.
    targets += (
        f"\n# cfs.py: {name} ({cpu})\n"
        "if (NOT CFE_EDS_ENABLED)\n"
        f"    list(APPEND {cpu}_APPLIST {name})\n"
        "endif()\n"
    )
    startup_entry = (
        f"    # cfs.py: {name}\n"
        f"    list(FIND ARGN {name} {symbol}_START_INDEX)\n"
        f"    if ({symbol}_START_INDEX GREATER -1)\n"
        f"        file(APPEND ${{STARTUP_FILE}}\n"
        f'            "CFE_APP, {name}, {symbol}_AppMain, {symbol}, 55, 32768, 0x0, 0;\\n"\n'
        "        )\n"
        "    endif()\n\n"
    )
    startup = startup.replace(
        "    # the rest of the apps can vary by config",
        startup_entry + "    # the rest of the apps can vary by config",
        1,
    )
    targets_file.write_text(targets)
    startup_file.write_text(startup)
    return app_dir


def make(root, config, goal):
    if not NAME_PATTERN.fullmatch(config):
        raise ValueError(f"Invalid configuration name: {config}")
    subprocess.run(["make", f"{config}.{goal}"], cwd=root, check=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    new = sub.add_parser("new", help="Generate and register a minimal app")
    new.add_argument("name", help="lowercase app name, for example sensor_reader")
    new.add_argument("--cpu", choices=CPUS, default="cpu1")
    new.add_argument("--build", action="store_true", help="Configure and build after generation")
    new.add_argument("--config", default="native_std")

    for command in ("build", "test", "run"):
        action = sub.add_parser(command)
        action.add_argument("--config", default="native_std")
        if command == "run":
            action.add_argument("--cpu", choices=CPUS, default="cpu1")

    args = parser.parse_args(argv)
    try:
        if args.command == "new":
            app_dir = create_app(ROOT, args.name, args.cpu)
            print(f"Created {app_dir.relative_to(ROOT)} and registered it on {args.cpu}")
            if args.build:
                make(ROOT, args.config, "prep")
                make(ROOT, args.config, "install")
        elif args.command == "build":
            make(ROOT, args.config, "prep")
            make(ROOT, args.config, "install")
        elif args.command == "test":
            make(ROOT, args.config, "runtest")
        else:
            if not NAME_PATTERN.fullmatch(args.config):
                raise ValueError(f"Invalid configuration name: {args.config}")
            run_dir = ROOT / f"build-{args.config}" / "exe" / args.cpu
            if not run_dir.is_dir():
                raise ValueError(f"Build output is missing: {run_dir}; run 'python3 cfs.py build' first")
            subprocess.run([f"./core-{args.cpu}"], cwd=run_dir, check=True)
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f"cfs.py: {exc}\n")


if __name__ == "__main__":
    main()
