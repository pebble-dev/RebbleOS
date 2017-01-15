import sys

print (sys.argv)


#MODESER = 0x6BC13C03
#OTYPER = 0x00008000
#OSPDR = 0x55010000
#PUPD = 0x00004054

MODESER = int(sys.argv[1], 16)
OTYPER = int(sys.argv[2], 16)
OSPDR = int(sys.argv[3], 16)
PUPD = int(sys.argv[4], 16)
IDR = int(sys.argv[5], 16)
ODR = int(sys.argv[6], 16)

print "-----------------------------------------------------------"
print "%-7s | %-10s | %-4s | %-6s | %-4s | %-4s | %-4s |" % ("PIN", "Mode", "Type", "Speed", "Pull", "In", "Out" )
print "-----------------------------------------------------------"

MODESER_ARR = ["IN", "OUT", "AF", "Analog"]
OTYPER_ARR = ["PP", "OD"]
OSPDR_ARR = ["2MHZ", "25MHZ", "50MHZ", "100MHZ"]
PUPD_ARR = ["None", "P-Up", "P-Dn"]
xDR_ARR = ["Low", "High"]

for x in range(0, 16):
	m = MODESER >> (2 * x) & 3
	o = OTYPER >> (x) & 1
	s = OSPDR >> (2 * x) & 3
	p = PUPD >> (2 * x) & 3
	idr = IDR >> (x) & 1
	odr = ODR >> (x) & 1
	
	func = MODESER_ARR[m]      
        ptype = OTYPER_ARR[o]
        spd = OSPDR_ARR[s]
        pupd = PUPD_ARR[p]  
        idrr = xDR_ARR[idr]
        odrr = xDR_ARR[odr]        
        
        print "GPIO_%2d | %-10s | %-4s | %-6s | %-4s | %-4s | %-4s |" % (x, func, ptype, spd, pupd, idrr, odrr)
