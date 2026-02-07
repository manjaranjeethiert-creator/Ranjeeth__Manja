from tkinter import *
ranjeeth_root=Tk()
#widthXheight -->gives size of window-->syntax"withXheight"
ranjeeth_root.geometry("720x720")
#creating min size of window while opening syntax->width,height
ranjeeth_root.minsize(400,400)
#creeating title of page
ranjeeth_root.title("InBody")
photo=PhotoImage(file="images.png")
inbody=Label(image=photo)
#pack contain side ->left,right,top,bottom
#anchor-->sw,se,ne,nw
inbody.pack()
#code goes here
ranjeeth_root.mainloop()
