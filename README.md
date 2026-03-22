# **KLC: KNOB-Lang-Compiler**

### *What is KNOB?* : **KNOB** is **K**ompiled **NOB**: k**N**ob-**O**riented-**B**inary
- This is a simple compiler project of mine to compile the KNOB (.knv) language into asm - a language I'm in the process of creating.
*Why **.knv**?*: Yes, *.knb* would've made more sense BUT... everyone knows that the true perfect language will always prioritize ergonomics over reliability and standards that make sense. With that in mind, you *should* know it's easier to hit 'v' from the home-row than 'b' - thank me later.

### General language quirks and features:
- Delimited language - ";"
- Static types (for now) (maybe dynamic later) 
- Common, well known practices possible (ideally)
- Kewords and Syntax stuffs:
	- Primitives: **int str char float double bool**
		- #### Keword-Specific Implementation:
			- int: tbd,
			- str: tbd,
			- char: tbd
			- float: tbd
			- double: tbd
			- bool: tbd
	- *return*: returns  
	- *func*: function
- More to be added here... KLC: KNOB-Lang-Compiler


- **If you would like to run the compiler in its current state:**
```bash
#Build
cmake -S . -B build/
cd ./build
cmake --build .

#Run
./klc <FILE.knv>
```

- **If you would like to build the a.s asm file:**
```bash
clang -c -g -o a.o a.s && ld -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -o out a.o && ./out
$$```
