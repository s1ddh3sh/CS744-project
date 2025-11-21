import re

def extract_avg_util(filename):
    util_values = []

    with open(filename, "r") as f:
        for line in f:
            # Look for the %util column value at the end of the device line
            # Typically the device line starts with something like "nvme0n1p5"
            if re.match(r'^\S+', line) and '%util' not in line and not line.startswith("Device"):
                parts = line.split()
                try:
                    util = float(parts[-1])  # %util is the last column
                    util_values.append(util)
                except ValueError:
                    pass

    if util_values:
        avg = sum(util_values) / len(util_values)
        return avg, util_values
    else:
        return None, []


if __name__ == "__main__":
    filename = "results/put_all/16/iostat.txt"  # change to your file
    avg, values = extract_avg_util(filename)

    if avg is not None:
        # print("Found %util values:", values)
        print("Average %util: {:.2f}".format(avg))
    else:
        print("No %util values found.")
