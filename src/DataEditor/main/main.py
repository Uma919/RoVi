import os
import csv
import glob
import folium
from folium.plugins import HeatMap

def getPath(rlapath):
    dire = os.path.dirname(os.path.abspath(__file__))
    path = os.path.normpath(os.path.join(dire, rlapath))
    return path 

if __name__ == '__main__':
    #/ 慣性計測データ用出力CSV作成 /#
    output_sens_csv_path = getPath('../output/output_sensor.csv')
    with open(output_sens_csv_path, 'w', newline="") as f:
        writer = csv.writer(f)
        writer.writerow(['Lng[deg]', 'Lat[deg]', 'AccX', 'AccY', 'AccZ', 'GyroX', 'GyroY', 'GyroZ'])

    input_sens_csv_path = getPath('../input/sens_*.csv')
    sens_csv_files = glob.glob(input_sens_csv_path) #入力フォルダ内ファイルパスの取得
    for csv_file in sens_csv_files:
        loc_lst = []

        # 入力CSV読込みとデータ整形 #
        with open(csv_file) as f:
            reader = csv.reader(f)
            for i, r in enumerate(reader):
                if i == 0:
                    loc_lst.append([r[:2], [r]])
                elif i != 0 and (r[0] == loc_lst[-1][0][0]) and (r[1] == loc_lst[-1][0][1]):
                    loc_lst[-1][1].append(r)
                else:
                    loc_lst.append([r[:2], [r]])

        # 内挿と出力用CSV書込み #
        with open(output_sens_csv_path, 'a', newline="") as f:
            writer = csv.writer(f)
            idx_end = len(loc_lst) - 1
            for i, l in enumerate(loc_lst):
                if i != idx_end:
                    if float(loc_lst[i+1][0][1]) - float(l[0][1]) != 0:
                        x_dt = (float(loc_lst[i+1][0][1]) - float(l[0][1]))/len(l[1])
                        for j, e in enumerate(l[1]):
                            x = float(l[0][1]) + (j * x_dt)
                            y = float(l[0][0]) + ((float(loc_lst[i+1][0][0]) - float(l[0][0])) * (x - float(l[0][1])) / (float(loc_lst[i+1][0][1]) - float(l[0][1])))
                            e[0] = x
                            e[1] = y
                            writer.writerow(e)
                    else:
                        y_dt = (float(loc_lst[i+1][0][0]) - float(l[0][0]))/len(l[1])
                        for j, e in enumerate(l[1]):
                            x = float(l[0][1])
                            y = float(l[0][0]) + (j * y_dt)
                            e[0] = x
                            e[1] = y
                            writer.writerow(e)
    
    stp_pnt = []
    with open(output_sens_csv_path) as f:
        reader = csv.reader(f)
        header = next(reader)
        stp_pnt = [[float(r[1]), float(r[0]), abs(float(r[5]))] for r in reader if abs(float(r[5])) >= 130]
    
    #/ 道路異常データ用出力CSV作成 /#
    output_repo_csv_path = getPath('../output/output_report.csv')
    with open(output_repo_csv_path, 'w', newline="") as f:
        writer = csv.writer(f)
        writer.writerow(['Lat[deg]', 'Lng[deg]', 'Details'])

    rep_pnt = []
    input_repo_csv_path = getPath('../input/repo_*.csv')
    repo_csv_files = glob.glob(input_repo_csv_path) #入力フォルダ内ファイルパスの取得
    for csv_file in repo_csv_files:
        # 入力CSV読込み #
        with open(csv_file) as f:
            reader = csv.reader(f)
            for r in reader:
                rep_pnt.append(r)

        # 内挿と出力用CSV書込み #
        with open(output_repo_csv_path, 'a', newline="") as f:
            writer = csv.writer(f)
            for l in rep_pnt:
                writer.writerow(l)

    # RoViマップファイル作成 #
    output_html_path = getPath('../output/index.html')
    map = folium.Map(location=[33.8811154,130.7109263], zoom_start=14)
    HeatMap(stp_pnt, 
            min_opacity=0.5, 
            radius=3, 
            blur=4, 
            gradient={0.2:'blue', 0.4:'lime', 0.6:'yellow', 0.8:'red'}).add_to(map)

    for pnt in rep_pnt:
        if pnt[2] == "step":
            folium.Marker(location=pnt[:2],
                          popup=pnt[2],
                          icon=folium.Icon(color="red", icon="info-sign")).add_to(map)
        elif pnt[2] == "grass":
            folium.Marker(location=pnt[:2],
                          popup=pnt[2],
                          icon=folium.Icon(color="green", icon="info-sign")).add_to(map)
    map.save(output_html_path)