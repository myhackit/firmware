
import math

A=64.4 # KB Height to hinge
B=53.0 # KB Width to hinge
X=15.5*25.4 # Elbow to home row
Y=17.25*25.4 # Elbow to Elbow
H=math.sqrt((X+A)**2 + B**2 - (Y/2)**2) # Hinge to chest
J=math.sqrt((X+A)**2 + B**2)  # Elbow to hinge
a=math.degrees(math.asin(B/J)) 
b=math.degrees(math.asin(H/J))
c=90-b-a
z=c*2 # Hinge separation angle

print A,B,X,Y,H,J,a,b,c,z
print "Forearm length: %s cm %s in " % (X, X/25.4)
print "Elbow width: %s cm %s in" % (Y, Y/25.4)
print "Distance hinge to body center: %s cm %s in" % (H, H/25.4)
print "Hinge angle: %s deg" % z
print "Arm angle: %s deg" % ((180.0-z)/2)
