# **KLC: KNOB-Lang-Compiler**

# *What is KNOB?* : **KNOB** is **K**ompiled **NOB**: k**N**ob-**O**riented-**B**inary
- Simple toy compiler to compile the KNOB (.knv) language into asm.
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


```bash
# asm build macos
 -o test.o test.s && ld -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -o test test.o && ./test
$$```
