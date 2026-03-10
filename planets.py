def alien_greeting (planet):
    print("Hello, ", planet, "!",sep='')
def ai_greeting (language, version):
    print("Welcome to ", language, " ", version, "!", sep='')

language = "Java"
v = 23

alien_greeting("Saturn")
ai_greeting("Python", 3.0)
print("What a nice interaction!")
ai_greeting(language, v)
print("You are confusing...")
