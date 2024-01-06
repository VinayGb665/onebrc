import random

def gen_random():
    randomCity = chr(random.randint(65,90));
    multiplier = 10 if random.randint(0,100) % 2 == 0 else 100;
    randomTemperature = round(random.uniform(-1, 1)*multiplier, 3);
    return f"{randomCity};{randomTemperature}"

def run(n=10**6):
    f = open("data-smol.txt", "w");
    for _ in range(0, n):
        f.write(f"{gen_random()}\n");

run(10**7);
