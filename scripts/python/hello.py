from colorama import Fore, Back, Style, init

# Initialize colorama. The autoreset=True argument makes it automatically
# reset the color after each print statement.
init(autoreset=True)

# --- Example Usage ---

# Fore sets the foreground (text) color
print(Fore.RED + "This text is red.")
print(Fore.GREEN + "This text is green.")

# Back sets the background color
print(Back.YELLOW + "This text has a yellow background.")

# Style can be used to change the text style
print(Style.BRIGHT + Fore.CYAN + "This is bright cyan text!")

# You can combine them all
print(Fore.MAGENTA + Back.WHITE + Style.DIM + "This is dim magenta text on a white background.")

# Because we used init(autoreset=True), the color resets automatically!
print("This text is back to the default color.")