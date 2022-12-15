n=7
k=3

print("A0: ")
a0 = []
for m in range(0,int((n-3)/2+1)):
	a0.append(((2*m+1)*k-1)%n)
print(a0)

a1 = []
print("A1: ")
for l in range(0,int((n-3)/2+1)):
	a1.append((n-(2*l+1)*k-1)%n)
print(a1)

b0 = []
print("B0: ")
for m in range(1,int((n-1)/2+1)):
	b0.append((2*m*k-1)%n)
print(b0)

b1 = []
print("B1: ")
for l in range(1,int((n-1)/2+1)):
	b1.append((n-2*l*k-1)%n)
print(b1)