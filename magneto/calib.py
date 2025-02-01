#!/usr/bin/env python3
import csv
import io
import os
import subprocess

def write_csv_no_trailing_newline(rows, filename):
    """
    Write rows (a list of lists) to a CSV file without a trailing newline.
    """
    output = io.StringIO()
    writer = csv.writer(output, lineterminator='\n')
    for row in rows:
        writer.writerow(row)
    csv_string = output.getvalue()
    # Remove the trailing newline if present.
    if csv_string.endswith('\n'):
        csv_string = csv_string[:-1]
    with open(filename, 'w', newline='') as f:
        f.write(csv_string)

def split_calibration(input_file, acc_file, mag_file, max_rows=300):
    """
    Reads the input CSV file and collects the first max_rows valid rows,
    splitting each row into two parts:
      - The first three values go to the accelerometer file.
      - The next three values go to the magnetometer file.
    """
    rows_acc = []
    rows_mag = []
    with open(input_file, 'r', newline='') as infile:
        reader = csv.reader(infile, skipinitialspace=True)
        for row in reader:
            if not any(field.strip() for field in row):
                continue
            if len(rows_acc) < max_rows:
                rows_acc.append(row[:3])
                rows_mag.append(row[3:6])
            else:
                break

    write_csv_no_trailing_newline(rows_acc, acc_file)
    write_csv_no_trailing_newline(rows_mag, mag_file)
    print(f"Processed {len(rows_acc)} rows (maximum {max_rows} rows).")

def run_magneto(input_csv, reject_threshold, hm, output_csv):
    """
    Runs magneto.exe with the given parameters.
    The command line for magneto.exe is:
         magneto.exe <input_csv> <reject_threshold> <Hm> <output_csv>
    """
    cmd = ["magneto.exe", input_csv, reject_threshold, hm, output_csv]
    print("Running command:", " ".join(cmd))
    # run and wait (raise an error if magneto.exe fails)
    subprocess.run(cmd, check=True)
    print(f"Finished processing {input_csv}.\n")

if __name__ == "__main__":
    # Define folder names
    input_dir = "input"
    output_dir = "output"

    # Build full file paths.
    input_filename  = os.path.join(input_dir, "calib.csv")
    acc_filename    = os.path.join(input_dir, "out_acc.csv")
    mag_filename    = os.path.join(input_dir, "out_mag.csv")
    # Choose output filenames for the corrected data.
    corr_acc_file   = os.path.join(output_dir, "corr_acc.csv")
    corr_mag_file   = os.path.join(output_dir, "corr_mag.csv")

    # Split the calibration file into separate files.
    split_calibration(input_filename, acc_filename, mag_filename)

    # Choose parameters for magneto.exe.
    # (For example, reject_threshold and Hm may be set to "0" to use defaults.)
    reject_threshold = "0"
    hm = "0"

    # Run magneto.exe on the accelerometer data.
    run_magneto(acc_filename, reject_threshold, hm, corr_acc_file)

    # Run magneto.exe on the magnetometer data.
    run_magneto(mag_filename, reject_threshold, hm, corr_mag_file)

    print(f"Calibration data has been processed.")
    print(f"Corrected accelerometer data is in: {corr_acc_file}")
    print(f"Corrected magnetometer data is in: {corr_mag_file}")
