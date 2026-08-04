import os
from pathlib import Path
import yaml

def prepare_LTE_demo(model_path: Path, bc_demo_file: Path) -> None:
    demos_file = model_path / "demos.yml"
    try:
        with demos_file.open("r", encoding="utf-8") as f:
            demo_dict = yaml.safe_load(f) or {}

        if not isinstance(demo_dict, dict):
            raise ValueError
    except (OSError, ValueError, yaml.YAMLError) as e:
        print(f"Error loading user's demos.yml file: {e}.")
        print("It is probably broken. Replacing.")
        demo_dict = {}

    demos = demo_dict.get("demos")
    if not isinstance(demos, dict):
        print("Demos section is missing or invalid. Creating from scratch.")
        demo_dict["demos"] = {}
        demos = demo_dict["demos"]

    bc_demo_file_issue = False
    try:
        with bc_demo_file.open("r", encoding="utf-8") as f:
            bc_demo = yaml.safe_load(f)
        if not isinstance(bc_demo, dict):
            bc_demo_file_issue = True
    except Exception as e:
        bc_demo_file_issue = True

    if bc_demo_file_issue:
        print(f"BC Demo file is missing or invalid. User will \
                have to create one manually from the Overseer settings")
        with open(demos_file, "w") as f:
            yaml.safe_dump(demo_dict, f)
        return

    bc_lte_dict = demos.get("bc_lte")
    if not isinstance(bc_lte_dict, dict):
        print("Demos dict has a missing or invalid BC-LTE entry. Creating one.")
        demos["bc_lte"] = bc_demo

    with open(demos_file, "w") as f:
        yaml.safe_dump(demo_dict, f)

def prepare_model_preset(model_path: Path, sim_path: str) -> None:
    params_file = model_path / "data" / "params.yml"

    try:
        with params_file.open("r", encoding="utf-8") as f:
            presets_dict = yaml.safe_load(f) or {}
        presets = presets_dict["presets"]
        if not isinstance(presets, dict):
            raise ValueError
    except (OSError, ValueError, yaml.YAMLError) as e:
        print(f"Error loading user's params.yml file: {e}")
        print("It is probably broken. Replacing.")
        presets_dict = {"presets": {}}
        presets = presets_dict["presets"]

    default_preset = presets.get("default_preset")
    if not isinstance(default_preset, dict):
        default_preset = {}
        presets["default_preset"] = default_preset

    default_preset["name"] = "default_preset"
    params = default_preset.get("params")
    if not isinstance(params, dict):
        params = {}
        default_preset["params"] = params

    params["exe_path"] = str(sim_path)

    with open(params_file, "w") as f:
        yaml.safe_dump(presets_dict, f)
    return

if __name__ == "__main__":
    bc_demo_file = Path(os.environ["BC_DEMO_FILE"])
    model_path = Path(os.environ["MODEL_PATH"])
    sim_path = os.environ["SIM_PATH"]

    prepare_LTE_demo(model_path, bc_demo_file)
    prepare_model_preset(model_path, sim_path)
