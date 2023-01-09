
#!/usr/bin/env python
# Implementation of collaborative filtering recommendation engine


from recommendation_data import dataset
from math import sqrt

#funzione confronta 2 persone per vedere quanto simili -> distanza euclidea
def similarity_score(person1, person2):
	
	# Returns ratio Euclidean distance score of person1 and person2 

	both_viewed = {}		# To get both rated items by person1 and person2

	for item in dataset[person1]:
		if item in dataset[person2]:
			both_viewed[item] = 1

		# Conditions to check they both have an common rating items	
		if len(both_viewed) == 0:
			return 0

		# Finding Euclidean distance 
		sum_of_eclidean_distance = []	

		for item in dataset[person1]:
			if item in dataset[person2]:
				sum_of_eclidean_distance.append(pow(dataset[person1][item] - dataset[person2][item],2))
		sum_of_eclidean_distance = sum(sum_of_eclidean_distance)

		return 1/(1+sqrt(sum_of_eclidean_distance))


def pearson_correlation(person1, person2):

	# To get both rated items
	both_rated = {}
	for item in dataset[person1]:
		if item in dataset[person2]:
			both_rated[item] = 1

	number_of_ratings = len(both_rated)		
	
	# Checking for number of ratings in common
	if number_of_ratings == 0:
		return 0

	# Add up all the preferences of each user
	person1_preferences_sum = sum([dataset[person1][item] for item in both_rated])
	person2_preferences_sum = sum([dataset[person2][item] for item in both_rated])

	# Sum up the squares of preferences of each user
	person1_square_preferences_sum = sum([pow(dataset[person1][item],2) for item in both_rated])
	person2_square_preferences_sum = sum([pow(dataset[person2][item],2) for item in both_rated])

	# Sum up the product value of both preferences for each item
	product_sum_of_both_users = sum([dataset[person1][item] * dataset[person2][item] for item in both_rated])

	# Calculate the pearson score
	numerator_value = product_sum_of_both_users - (person1_preferences_sum*person2_preferences_sum/number_of_ratings)
	denominator_value = sqrt((person1_square_preferences_sum - pow(person1_preferences_sum,2)/number_of_ratings) * (person2_square_preferences_sum -pow(person2_preferences_sum,2)/number_of_ratings))
	if denominator_value == 0:
		return 0
	else:
		r = numerator_value/denominator_value
		return r 

def most_similar_users(person, number_of_users):
	# returns the number_of_users (similar persons) for a given specific person.
	scores = [(pearson_correlation(person,other_person),other_person) for other_person in dataset if  other_person != person ]
	
	# Sort the similar persons so that highest scores person will appear at the first
	scores.sort()
	scores.reverse()
	return scores[0:number_of_users]

def user_reommendations(person):

	# Gets recommendations for a person by using a weighted average of every other user's rankings
	totals = {}
	simSums = {}
	rankings_list =[]
	for other in dataset:
		# don't compare me to myself
		if other == person:
			continue
		sim = pearson_correlation(person,other)
		#print(">>>>>>>",sim)

		# ignore scores of zero or lower
		if sim <=0: 
			continue
		for item in dataset[other]:

			# only score movies i haven't seen yet
			if item not in dataset[person] or dataset[person][item] == 0:

			# Similrity * score
				totals.setdefault(item,0)
				totals[item] += dataset[other][item]* sim
				# sum of similarities
				simSums.setdefault(item,0)
				simSums[item]+= sim

		# Create the normalized list

	rankings = [(total/simSums[item],item) for item,total in totals.items()]
	rankings.sort()
	rankings.reverse()
	# returns the recommended items
	recommendataions_list = [recommend_item for score,recommend_item in rankings]
	for i in range(len(rankings)):
		print(rankings[i])
	return recommendataions_list
		
def calculateSimilarItems(prefs,n=10):

        # Create a dictionary of items showing which other items they
        # are most similar to.
        result={}
        # Invert the preference matrix to be item-centric
        itemPrefs=transformPrefs(prefs)
        c=0
        for item in itemPrefs:
                # Status updates for large datasets
                c+=1
                if c%100==0: print("%d / %d" % (c,len(itemPrefs)))
                # Find the most similar items to this one
                scores=topMatches(itemPrefs,item,n=n,similarity=sim_distance)
                result[item]=scores
        return result

def stimaValutazione():
	for user in dataset:
		voto_medio = 0.0
		media = 0.0
		n = 0
		numeratore = 0.0
		denominatore = 0.0
		print(f"\nUtente {user}")

		#voto medio dell'utente
		for item in dataset[user]:
			voto_medio += dataset[user][item]
		voto_medio = voto_medio / len(dataset[user])

		print(f"Voto medio: {round(voto_medio, 2)}")

		#per ogni film votato da questo utente
		for item in dataset[user]:
			for people in dataset:
				if people == user:
					continue

				#altri utenti che hanno votato stesso film
				if item in dataset[people]:
					#calcolo similarità
					sim = pearson_correlation(user, people)

					#calcolo media di questo utente
					for people_item in dataset[people]:
						media += dataset[people][people_item]
					media = media / len(dataset[people])

					#formula stima di valutazione
					numeratore += sim * (dataset[people][people_item] - media)
					denominatore += sim 
			
			dataset[user][item] = media + (numeratore/denominatore)

			print(f"Stima valutazione, raccomandazione per prodotto {item}: {round(dataset[user][item], 2)}")


#ESERCIZIO 1
#punto a
print("--CORRELAZIONE DI PEARSON--")
for people in dataset:
	if people == 'Toby':
		continue
	sim = pearson_correlation('Toby', people)

	print(f"\n{people} Similarity: {round(sim, 2)}")

	for item in dataset[people]:
		if item not in dataset['Toby']:
			print(f"Film: {item}, Voto: {round(dataset[people][item], 2)}, Similarity film: {round(dataset[people][item] * sim, 2)} ")

consigliati = user_reommendations('Toby')
print("\nFilm consigliati per Toby: ")
print(', '.join(consigliati))



#punto b
print("\n--DISTANZA EUCLIDEA--")
for people in dataset:
	if people == "Toby":
		continue
	sim = similarity_score('Toby', people)

	print(f"\n{people} Similarity: {round(sim, 2)}")

	for item in dataset[people]:
		if item not in dataset['Toby']:
			print(f"Film: {item}, Voto: {round(dataset[people][item], 2)}, Similarity film: {round(dataset[people][item] * sim, 2)} ")



consigliati = user_reommendations('Toby')
print("\nFilm consigliati per Toby: ")
print(', '.join(consigliati))

#punto c
stimaValutazione()



