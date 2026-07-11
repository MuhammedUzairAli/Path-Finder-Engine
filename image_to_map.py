import csv
import numpy as np
from PIL import Image
from glob import glob
import render as rend

def pxlCheck(pxl): #a function to map pxls to their symbols
    match (int(pxl[0]),int(pxl[1]),int(pxl[2])):
        #for checking gray all colors should be somewhat within eahc others range and the range for them is 50 to 201
        case (r,g,b) if r >= 126 and r-g >= 30 and r - b >= 30 and abs(g-b) <=25: #reds the reder the more risk
            rLvl = int(1 + (((r-max(g,b)) - 50) * 4) / 205)  # ie max rlvl or base rLvl with diff as 255 will be 5 and 50 as 1, int for simplicity
            return str(rLvl)+'N'
        case (r,g,b) if abs(r-g) <= 20 and abs(g-b) <=20 and 40<= r <=200 and 40<= g <=200 and 40<= b <=200 : 
            return 'N'
        case (r,g,b) if b >= 128 and b-g >= 50 and b - r >= 50 and abs(g-r) <=10: #for blue which are hubs
            return 'H'
        case (r,g,b) if abs(r-g) <= 10 and abs(g-b) <=10 and max(r,g,b) < 40: # for black which are vils
            return 'V'
        case _: #if it is nothing else it is automatically assumed blocked road
            return 'B'
def main ():
    imgs = glob('terrain images/*.jpg') + glob('terrain images/*.png')
    print(imgs)
    for im in imgs:
        img = np.array(Image.open(im))
        with open("terrain.csv",'w',newline='') as file:
            wrtr = csv.writer(file)
            for row in img:    
                line = [pxlCheck(pxl) for pxl in row]
                line.append('') #for trailing commas
                wrtr.writerow(line)
        rend.rend()

if __name__ == '__main__':
    main()