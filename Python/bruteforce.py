from Crypto.Cipher import AES
from binascii import unhexlify
import itertools

def read_file(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()

    ciphertext = None
    decrypted_text = None

    for line in lines:
        if line.startswith("Ciphertext:"):
            ciphertext = line.split(":")[1].strip().replace(" ", "")
        elif line.startswith("Decrypted Text:"):
            decrypted_text = line.split(":")[1].strip().replace(" ", "")

    if not ciphertext or not decrypted_text:
        raise ValueError("File format is invalid. Ensure it contains 'Ciphertext' and 'Decrypted Text'.")

    return unhexlify(ciphertext), unhexlify(decrypted_text)

def brute_force_aes(ciphertext, expected_plaintext):
    with open("tried_keys.txt", "w") as log_file:
        for i, key_tuple in enumerate(itertools.product(range(256), repeat=16)):
            key = bytes(key_tuple)
            log_file.write(f"Trying key {i}: {key.hex()}\n")
            print(f"Trying key {i}: {key.hex()}", end="\r")
            cipher = AES.new(key, AES.MODE_ECB)
            decrypted_text = cipher.decrypt(ciphertext)

            if decrypted_text == expected_plaintext:
                print()  
                return key
    return None


if __name__ == "__main__":
    file_path = "logs/logs_aes128/aes128rfida/aes128rfid_16.txt" 

    try:
        ciphertext, expected_plaintext = read_file(file_path)
        print("Starting brute force...")

        key = brute_force_aes(ciphertext, expected_plaintext)

        if key:
            print(f"Key found: {key.hex()}")
        else:
            print("Key not found.")

    except Exception as e:
        print(f"Error: {e}")
