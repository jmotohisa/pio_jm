#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import csv
import argparse
import os
import xml.etree.ElementTree as ET
from datetime import datetime


def get_args():
    parser = argparse.ArgumentParser(description='Convert myGPS cvs to GPX')
    parser.add_argument("csvfile")
    parser.add_argument(
        "--garmin",
        action="store_true",
        help="Output Garmin TrackPointExtension")
    args = parser.parse_args()

    return args

def main():
    args=get_args()
    csvfile=args.csvfile
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
    
    with open(csvfile, newline="", encoding="utf-8") as f:
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

    dname,bname= os.path.split(csvfile)
    bname0= os.path.splitext(bname)[0]
    gpxfile = os.path.join(dname, bname0+'.gpx')

    ET.ElementTree(gpx).write(
        gpxfile,
        encoding="utf-8",
        xml_declaration=True
    )

if __name__=='__main__':
    main()

