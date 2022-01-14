import csv
import random

with open('test.csv', 'r') as file:
    f = csv.reader(file)
    data = []
    for row in f:
        data.append(row + [random.randint(1,8)])
    rows = data[1:]

with open('random.csv', 'w') as file_out:
   f = csv.writer(file_out)
   header = ['track_id','genre_id'] 
   f.writerow(header)
   f.writerows(rows)


