import numpy as np
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    args = parser.parse_args()

    filename = args.filename
    with open(filename) as f:
        content = f.readlines()

    content = content[6:-2]

    time_dict = {}
    for line in content:
        token = line.strip().split(' ')
        key = token[0]
        value = float(token[1])
        if key not in time_dict:
            time_dict[key] = [value]
        else:
            time_dict[key].append(value)

    integrate_time = np.array(time_dict['basic.integration'])
    raycast_time = np.array(time_dict['basic.raycasting'])
    print(integrate_time.mean())
    print(raycast_time.mean())
