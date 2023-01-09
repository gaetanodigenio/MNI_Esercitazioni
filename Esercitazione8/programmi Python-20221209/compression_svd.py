from matplotlib.image import imread
import matplotlib.pyplot as plt
import numpy as np

A = imread('cane.png')
# A è una matrice  M x N x 3, essendo un'immagine RGB
# A(:,:,1) Red A(:,:,2) Blue A(:,:,3) Green
# su una scala tra 0 e 1
print(A.shape)


X = np.mean(A,-1); # media lungo l'ultimo asse, cioè 2
img = plt.imshow(X)
img.set_cmap('gray')
plt.axis('off')
plt.show()


# If full_matrices=True (default), u and vT have the shapes (M, M) and (N, N), respectively.
# Otherwise, the shapes are (M, K) and (K, N), respectively, where K = min(M, N).
U, S, VT = np.linalg.svd(X,full_matrices=False)
print(S[100:105])
S = np.diag(S)


j=0

for r in (5,20,100):
	Xapprox = U[:,:r] @ S[0:r,:r] @ VT[:r,:]

	numeratore = 0
	denominatore = 0

	for i in range(0, r):
		numeratore += S[i][i]

	
	for i in range(0, S.shape[0]): #valori singolari
		denominatore += S[i][i]

	energia = numeratore/denominatore


	plt.figure(j+1)
	j +=1
	img = plt.imshow(Xapprox)
	img.set_cmap('gray')
	plt.axis('off')
	plt.title(f'r = {r}, energia: {round(energia, 2)}')
	plt.show()
