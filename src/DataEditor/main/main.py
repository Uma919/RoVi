import os
import csv
import glob

def getPath(rlapath):
    dire = os.path.dirname(os.path.abspath(__file__))
    path = os.path.normpath(os.path.join(dire, rlapath))
    return path 

if __name__ == '__main__':
    # 出力用CSV作成 #
    output_file_path = getPath('../output/output.csv')
    with open(output_file_path, 'w', newline="") as f:
        writer = csv.writer(f)
        writer.writerow(['Sats[-]', 'Lng[deg]', 'Lat[deg]', 'Alt[m]', 'Speed[km/h]', 'Course[deg]', 'AccX', 'AccY', 'AccZ', 'GyroX', 'GyroY', 'GyroZ'])

    input_files_path = getPath('../input/*.csv')
    csv_files = glob.glob(input_files_path) #入力フォルダ内ファイルパスの取得
    for csv_file in csv_files:
        loc_lst = []

        # 入力用CSV読込みとデータ整形 #
        with open(csv_file) as f:
            reader = csv.reader(f)
            for i, r in enumerate(reader):
                if i == 0:
                    loc_lst.append([r[1:3], [r]])
                elif i != 0 and (r[1] == loc_lst[-1][0][0]) and (r[2] == loc_lst[-1][0][1]):
                    loc_lst[-1][1].append(r)
                else:
                    loc_lst.append([r[1:3], [r]])

        # 内挿と出力用CSV書込み #
        with open(output_file_path, 'a', newline="") as f:
            writer = csv.writer(f)
            idx_end = len(loc_lst) - 1
            for i, l in enumerate(loc_lst):
                if i != idx_end:
                    if float(loc_lst[i+1][0][1]) - float(l[0][1]) != 0:
                        x_dt = (float(loc_lst[i+1][0][1]) - float(l[0][1]))/len(l[1])
                        for j, e in enumerate(l[1]):
                            x = float(l[0][1]) + (j * x_dt)
                            y = float(l[0][0]) + ((float(loc_lst[i+1][0][0]) - float(l[0][0])) * (x - float(l[0][1])) / (float(loc_lst[i+1][0][1]) - float(l[0][1])))
                            e[1] = x
                            e[2] = y
                            writer.writerow(e)
                    else:
                        y_dt = (float(loc_lst[i+1][0][0]) - float(l[0][0]))/len(l[1])
                        for j, e in enumerate(l[1]):
                            x = float(l[0][1])
                            y = float(l[0][0]) + (j * y_dt)
                            e[1] = x
                            e[2] = y
                            writer.writerow(e)
    