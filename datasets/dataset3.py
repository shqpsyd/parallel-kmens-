import sys
import random
A = int(sys.argv[1])#amount of nodes
B = int(sys.argv[2])#attributes of nodes
C = int(sys.argv[3])#claster
D = int(sys.argv[4])#max iters
E = int(sys.argv[5])#set to be 0


nodes = []
for i in range(A):
	temp = []
	for j in range(B):
		temp.append(random.random())
	nodes.append(temp)
with open ("dataset3.txt",'w') as file :
	file.write(" ".join(sys.argv[1:]))
	file.write("\n")
	for node in nodes:
		line = ""
		for i in node:
			line += str(i)
		file.write(line + '\n')


