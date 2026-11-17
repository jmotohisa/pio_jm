#!/usr/bin/env python3

import csv
import argparse
import xml.etree.ElementTree as ET
from datetime import datetime


parser = argparse.ArgumentParser()
parser.add_argument("csvfile")
parser.add_argument("gpxfile")
parser.add_argument(
    "--garmin",
    action="store_true",
    help="Output Garmin TrackPointExtension"
)

args = parser.parse_args()


if args.garmin:

    gpx = ET.Element(
        "gpx",
        {
            "version": "1.1",
            "creator": "Arduino TinyGPSPlus Logger",
            "xmlns": "http://www.topografix.com/GPX/1/1",
            "xmlns:gpxtpx":
                "http://www.garmin.com/xmlschemas/TrackPointExtension/v1"
        }
    )

else:

    gpx = ET.Element(
        "gpx",
        {
            "version": "1.1",
            "creator": "Arduino TinyGPSPlus Logger",
            "xmlns": "http://www.topografix.com/GPX/1/1"
        }
    )


trk = ET.SubElement(gpx, "trk")
ET.SubElement(trk, "name").text = "GPS Track"

trkseg = ET.SubElement(trk, "trkseg")

with open(args.csvfile, newline="", encoding="utf-8") as f:

    reader = csv.DictReader(f)

    for row in reader:

        dt = datetime.strptime(
            f"{row['Date_UTC']} {row['Time_UTC']}",
            "%Y-%m-%d %H:%M:%S"
        )

        trkpt = ET.SubElement(
            trkseg,
            "trkpt",
            lat=row["Latitude"],
            lon=row["Longitude"]
        )

        ET.SubElement(
            trkpt,
            "ele"
        ).text = row["Altitude"]

        ET.SubElement(
            trkpt,
            "time"
        ).text = dt.strftime("%Y-%m-%dT%H:%M:%SZ")

        #
        # Garmin拡張
        #
        if args.garmin:

            ext = ET.SubElement(
                trkpt,
                "extensions"
            )

            tpx = ET.SubElement(
                ext,
                "{http://www.garmin.com/xmlschemas/TrackPointExtension/v1}"
                "TrackPointExtension"
            )

            # km/h -> m/s
            speed_ms = (
                float(row["Speed"]) / 3.6
            )

            ET.SubElement(
                tpx,
                "{http://www.garmin.com/xmlschemas/TrackPointExtension/v1}"
                "speed"
            ).text = f"{speed_ms:.3f}"

            ET.SubElement(
                tpx,
                "{http://www.garmin.com/xmlschemas/TrackPointExtension/v1}"
                "course"
            ).text = row["Course"]


ET.indent(gpx, space="  ")

ET.ElementTree(gpx).write(
    args.gpxfile,
    encoding="utf-8",
    xml_declaration=True
)