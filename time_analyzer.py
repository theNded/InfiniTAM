import numpy as np
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    args = parser.parse_args()

    filename = args.filename
    with open(filename) as f:
        content = f.readlines()

    content = content[6:-3]

    time_dict = {}
    for line in content:
        token = line.strip().split(' ')
        key = token[0]
        value = float(token[1])
        if key not in time_dict:
            time_dict[key] = [value]
        else:
            time_dict[key].append(value)

    for k, v in time_dict.items():
        print(k, np.array(v).mean())
