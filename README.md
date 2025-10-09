# SHAirDrop

It's like AirDrop, except you can only share small images, it's written in C, needs you to use a command line, will spuriously break and catch fire in unexpected circumstances (as a feature), and honestly just kind of hates you.

"ShairDrop" = Shit AirDrop or Shell AirDrop or Share Drop... I don't fucking know make it what you want. Who am I to tell you what to do?

## VERY GOOD DESIGN DOCUMENT

### Avatar Packet

- Width - 1 byte; 8 bits (max. 2^8 - 1 size; 255); boundscheck this!
- Height - 1 byte as well
- Channel count - 1 byte, can only be 1 (grayscale), 3 (RGB), or 4 (RGBA)
   - We will convert all lower chan. counts into RGBA!
- Pixel data itself
   - Will need to manip. this for the above effect
- Also very important is the size
   - Width  height  channel count
- And how big the packet is
   - 1 + 1 + 1 + (width  height  channel count)
- After further thinking, an opcode header makes a lot of sense as well
 - We might get a packet and a size but we have no fucking idea what the packet is meant for... How do we know what to do with it?
 - Do this later
 - I am hungry
 - :D

### Pipeline

 1. Load image using Raylib
 2. Extract packet contents
 3. Assemble packet
 4. Send
 5. Server receives image
 6. Server displays image
   - We never save it to filesystem though

### Notes

- Using TCP INET4 for simplicity
- sendall, recvall to ensure shit makes it across
- No network thread yet; will add tho

### Code Style

- PascalCase for functions and structs
- camelCase for variable names
- .clangd does everything else :)
