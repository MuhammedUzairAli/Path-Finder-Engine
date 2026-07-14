import subprocess as p
import matplotlib.pyplot as plt
import matplotlib.colors as clr
import matplotlib.patches as mp
import numpy as np
from matplotlib.ticker import MultipleLocator as Ml
import csv

def mapper(out,rows,cols,iter):
    start = 0 + iter * rows * cols
    return np.array([[int(out[j*cols+(start + i)]) for i in range(0,cols)] for j in range(0,rows)])

def rend ():
    path_finder = p.run("path_finder_engine.exe",stdout = p.PIPE ) #starts the executable
    myout  = path_finder.stdout.decode() #the output from path_finder_engine.exe is converted to string
    #the 1st 4 character contain info about the map
    numrow = ""
    numcol = ""
    numhub = ""
    numvil = ""
    i=0
    while(myout[i] != ","):
        numrow = numrow + myout[i] #concatanating all digits till , flag
        i+=1
    numrow = int(numrow) #convert to int
    i+=1 #skip , wich i is currently at
    while(myout[i] != ","):
        numcol = numcol + myout[i] 
        i+=1
    numcol = int(numcol) 
    i+=1
    while(myout[i] != ","):
        numhub = numhub + myout[i] 
        i+=1
    numhub = int(numhub)  
    i+=1
    while(myout[i] != ","):
        numvil = numvil + myout[i] 
        i+=1
    numvil = int(numvil) 
    i+=1 #skip last comma
    print(numrow)
    print(numcol)
    print(numhub)
    print(numvil)
    myout = myout[i:len(myout)] #striping the string of chars carrying rownum colnum hub num vilnum
    print(myout)
    old_map = mapper(myout,numrow,numcol,0) #get the oldmap with iter 0

    print(old_map)
    hubs = tuple(((i,j) for j in range(0, numrow) for i in range(0,numcol) if old_map[j,i]==3 ))
    vils = tuple(((i,j) for j in range(0, numrow) for i in range(0,numcol) if old_map[j,i]==4 ))

    print(hubs)
    print(vils)

    y = 0
    risk = 0
    road_data = []
    with open("terrain.csv","r") as terrain:
            content = csv.reader(terrain)
            for line in content:
                    print(line)
                    for i,val in enumerate(line):
                        if('N' in val.upper()):
                            try: #if a number is to left and right of n then risk is simply sum of that number
                                risk += abs(float(val.upper().split('N',1)[0]))
                            except:
                                pass
                            try: 
                                risk += abs(float(val.upper().split('N',1)[1])) #negative treated same as positive
                            except:
                                pass
                            road_data.append((i,y,risk))
                            risk = 0
                    y+=1
    road_data = tuple(road_data)
    print(road_data)
    risks = [val[2] for val in road_data if val[2] > 0]
    norm = clr.Normalize(vmin=min(risks) if risks else 0, vmax=max(risks) if risks else 1.0)

    overlay = np.zeros((numrow,numcol,4))
    overlay[..., 0] = 1.0 #the third dimension is for the color channels with red green blue and alpha as red is in index 0 of 3rd dimension I set it to 1

    for x,y,risk in road_data:
        if risk ==0:
            overlay[y,x,3]= 0.0 #ie alpha is zero and it is transparent
        else:
            overlay[y,x,3] = norm(risk)*0.8 + 0.2
    
    #code for making and formatting plot
    plt.ion()
    plt.show()
    labels = ("No road / terrain","Best path","Road","Hub","village")
    clrs = ("green","yellow","grey","blue","black")
    fig,ax = plt.subplots(figsize = (numcol,numrow))
    colrmap = clr.ListedColormap(clrs)
    img = ax.imshow(old_map,cmap=colrmap,origin = 'upper')
    ax.xaxis.set_major_locator(Ml(1 if numcol < 50 else int(numcol/10)))
    ax.yaxis.set_major_locator(Ml(1 if numrow < 50 else int(numrow/10)))
    font_size = max(3,12 * (10 / max(numrow,numcol))) #im taking 12 the normal size for 10 cols or 10 rows
    ovr = ax.imshow(overlay,origin = 'upper') #the overlay is made ontop of previous map with transparency values
    hubTxt = []
    vilTxt = []
    #this is for labeling hubs and villages
    i=0
    for i,(x,y) in enumerate(hubs):
        hubTxt.append(ax.text(x,y,f"H{i+1}", color='white', weight='bold',ha='center',va='center',fontsize =font_size))
    for i,(x,y) in enumerate(vils):
        vilTxt.append(ax.text(x,y,f"V{i+1}", color='white', weight='bold',ha='center',va='center',fontsize =font_size))
    
    patches = [mp.Patch(color = color , label = label) for label,color in zip(labels,clrs)] #gets the handles for the legend
    ax.legend(handles=patches, title="Map Features", loc="upper left", bbox_to_anchor=(1.05, 1))
    
    #this is for risk legend
    risk_alpha = {} #risk and corresponig alpha
    for x,y,r in road_data:
        if r > 0:
            risk_alpha[r]= overlay[y,x,3]
    sorted_risks = sorted(risk_alpha.keys())
    legend = ax.get_legend()
    risk_patches = [mp.Patch(color= (1.0,0.0,0.0, risk_alpha[r]),label=f"Risk {r}") for r in sorted_risks]
    ax.legend(handles=risk_patches, title="Road Risk Levels", loc="upper left",bbox_to_anchor=(1.05, 0.65)) #anchors it right below other legend
    ax.add_artist(legend) #makes prior legend reapear

    plt.title("map",fontweight="bold")

    if plt.fignum_exists(fig.number):
        plt.pause(0.1)

    while(plt.fignum_exists(fig.number)):
        if plt.waitforbuttonpress():
            break  

    print(numrow,numcol,numhub,numvil)
    if numhub == 0 or numvil == 0:
        print("no villages or hubs")
        return
    iter = 1 #iter is set to 1 as old map was taken before
    for i in range(1,numhub+1) :
        for j in range(1,numvil+1) :
            if myout[iter*numcol*numrow] == "7" :
                ax.set_title(f"no path from hub {i} to village {j}",fontweight="bold")
                img.set_data(old_map)
                if(plt.fignum_exists(fig.number)):
                    plt.pause(0.1)
                while(plt.fignum_exists(fig.number)):
                    if plt.waitforbuttonpress():
                        break               
            else:
                map = (mapper(myout,numrow,numcol,iter))
                print(map)
                ax.set_title(f"path from hub {i} to village {j}",fontweight="bold")
                img.set_data(map)
                activeHub = hubTxt[i-1]
                activeVil = vilTxt[j-1]
                if(plt.fignum_exists(fig.number)):
                    plt.pause(0.1)
                while(plt.fignum_exists(fig.number)):
                    activeHub.set_color('blue')
                    activeVil.set_color('black')
                    plt.pause(0.1)
                    activeHub.set_color('white')
                    activeVil.set_color('white')
                    if plt.waitforbuttonpress(timeout=0.3):
                        break
            print()
            iter += 1
    plt.ioff()
    plt.show()
    if(plt.fignum_exists(fig.number)):
        plt.pause(0.1)
        while(plt.fignum_exists(fig.number)):
            activeHub.set_color('blue')
            activeVil.set_color('black')
            plt.pause(0.1)
            activeHub.set_color('white')
            activeVil.set_color('white')
            if plt.waitforbuttonpress(timeout=0.3):
                break
    plt.close(fig)              
if __name__ == '__main__':
    rend()