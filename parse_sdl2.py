import sdl2.ext
import sdl2.ext.colorpalettes
from random import randint

sdl2.ext.init()

width = 240
height = 96
window = sdl2.ext.Window("Hello World!", size=(width, height))
window.show()

renderer = sdl2.ext.Renderer(window)

windowsurface = window.get_surface()
sdl2.ext.fill(windowsurface, sdl2.ext.colorpalettes.MONOPALETTE[0])

imgnum = 0

filename ="juno_sigrok_12m_10g2.csv" #no duplicates
#myc = f.readlines()
#myc.pop(0)
#myc.pop(0)
#hh = myc.pop(0)
#print (hh)
#print (hh[1:9], "we:",hh[9], "rs:",hh[10], "cs1:",hh[11], "cs2:",hh[12] )
x = 0
y1 = 0
y2 = 0
ctrl = 0
cmd = []

#The Display Data RAM stores pixel data for the LCD. It is 65-row by 132-column addressable array. Each pixel
#can be selected when the page and column addresses are specified. The 65 rows are divided into 8 pages of 8
#lines and the 9th page with a single line (DB0 only). Data is read from or written to the 8 lines of each page
#directly through DB0 to DB7. The display data of DB0 to DB7 from the microprocessor correspond to the LCD
#common lines as shown in Figure 6. The microprocessor can read from and write to RAM through the I/O buffer.
#Since the LCD controller operates independently, data can be written into RAM at the same time as data is being
#displayed without causing the LCD flicker

cmds = {
    'ae': "display on",
	'af': "display off", 
    '81': "set reference voltage",
}

def showcmd(val):
	if (val & 0xfe ) == 0xae:
		print ("cmd 0x%02x (on/off)" % val)
	elif (val & 0xfe ) == 0xa0:
		print ("cmd 0x%02x (ADC select)" % val)
	elif (val & 0xf0 ) == 0x40:
		print ("cmd 0x%02x (Initial display line)" % val)
	elif (val & 0xf0 ) == 0x20:
		print ("cmd 0x%02x (Regulator resistor select)" % val)
	elif (val & 0xf0 ) == 0xc0:
		print ("cmd 0x%02x (SHL select)" % val)
	elif val == 0xe2:
		print ("cmd 0x%02x (reset)" % val)
	elif (val == 0xf0 ) == 0x10 or (val == 0xf0 ) == 0x00:
		print ("cmd 0x%02x (set column address)" % val)
	else:
		print ("cmd 0x%02x" % val)

mycnt=0
mycntdata=0
page1 = 0
page2 = 0

cs1 = -1
cs2 = -1
cs1_pre = -1
cs2_pre = -1
cs1_state = None
cs2_state = None
with open(filename) as f:
	for i in f:
		ss = i.rstrip().split(",")
		if ss[0] == "Time":
			continue

		re = ss[9]
		rs = ss[10]
		cs1_pre = cs1
		cs1 = ss[11]
		cs2_pre = cs2
		cs2 = ss[12]
		
		if cs1_pre == "0" and cs1 == "1":
			cs1_state = 1
		elif cs1_pre == "1" and cs1 == "0":
			cs1_state = 0
		else:
			cs1_state = -1

		if cs2_pre == "0" and cs2 == "1":
			cs2_state = 1
		elif cs2_pre == "1" and cs2 == "0":
			cs2_state = 0
		else:
			cs2_state = -1
		
		if cs1_state != 1 and cs2_state != 1:
			continue

		if cs1_state==1:
			if re == "0" and rs == "1":
				val = 0
				for va in range(0,8):
					val += (int(ss[1+va])*(pow(2,va)))
				
				if (val == 0xb0 ) or val == (0xaf):
					print ("%d %d "% (mycnt, mycntdata))
					mycnt=0
					mycntdata=0
				if ("%02x" % val).startswith("b"):
					page1 = val & 0xf
					#print ("cmd 0x%02x" % val, "%4d" % val, "%8s" % ss[0], ss[1:9], "we:",ss[9], "rs:",ss[10], "cs1:",cs1, "cs2:",cs2, ctrl, "page1:", page1)			
					y1 = 0
				elif val not in [0x10, 0x00]:
					showcmd(val)
				mycnt +=1
			elif re == "1" and rs == "1":
				#print(".",end="")
				for px in range(0,8):
					renderer.draw_point([120 + y1,page1 * 8 +px], sdl2.ext.colorpalettes.MONOPALETTE[int(ss[1+px])])
					#renderer.draw_point([120 + y1,page1 * 8 +px], sdl2.ext.colorpalettes.WEBPALETTE[128*int(ss[1+px])])
				y1+=1
				mycntdata +=1
				mycnt +=1
				

		if cs2_state==1:
			if re == "0" and rs == "1":
				val = 0
				for va in range(0,8):
					val += (int(ss[1+va])*(pow(2,va)))
				if ("%02x" % val).startswith("b"):
					page1 = val & 0xf
					#print ("cmd 0x%02x" % val, "%4d" % val, "%8s" % ss[0], ss[1:9], "we:",ss[9], "rs:",ss[10], "cs1:",cs1, "cs2:",cs2, ctrl, "page2:", page2)
					y2 = 0
				#elif val not in [0x10, 0x00]:
				#	showcmd(val)
			elif re == "1" and rs == "1":
				for px in range(0,8):
					renderer.draw_point([y2,page1 * 8 +px], sdl2.ext.colorpalettes.MONOPALETTE[int(ss[1+px])])
				y2+=1
	
		renderer.present()

print ("END")
processor = sdl2.ext.TestEventProcessor()
processor.run(window)
sdl2.ext.quit()