#!/usr/bin/env python
import json
import sys
import os

upgrade_stream_types = {"satellite_delivery_descriptor": "sat",
                        "cable_delivery_descriptor": "cab",
                        "terrestrial_delivery_descriptor": "ter"}
config_mandatory_fields = ["general"]


def open_config(json_path):
    if not os.path.exists(json_path):
        return False, None
    ota_cfg = _read_config(json_path)
    status = _check_config(ota_cfg)
    return status, ota_cfg


def _check_config(data):
    if any(field in data for field in upgrade_stream_types)\
            and all(field in data for field in config_mandatory_fields):
        return True
    else:
        print >> sys.stderr, "No valid stream type field found in JSON file, aborting\n"
        return False


def _read_config(json_path):
    with open(json_path) as data_file:
        return json.load(data_file)
