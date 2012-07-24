
import math

A=65.0 # Height to hinge
B=60.0 # Width to hinge
X=15.5*25.4 # Elbow to home row
Y=17.25*25.4 # Elbow to Elbow
H=math.sqrt((X+A)**2 + B**2 - (Y/2)**2) # Hinge to chest
J=math.sqrt((X+A)**2 + B**2)  # Elbow to hinge
a=math.degrees(math.asin(B/J)) 
b=math.degrees(math.asin(H/J))
c=180-b-a-90
z=c*2 # Hinge separation angle

print A,B,X,Y,H,J,a,b,c,z
print "Forearm length: ", X, X/25.4
print "Elbow width: ", Y, Y/25.4
print "Distance hinge to body center: ", H, H/25.4
print "Hinge angle: ", z
print "Arm angle: ", (180.0-z)/2
