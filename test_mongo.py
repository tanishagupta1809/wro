from pymongo import MongoClient
client = MongoClient("mongodb+srv://tanisha:mongodb47@cluster0.duy9rx8.mongodb.net/?retryWrites=true&w=majority")
db = client["waste_db"]
bins = list(db["bins"].find())
print(bins)
