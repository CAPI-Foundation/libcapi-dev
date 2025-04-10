# Create Application Programmable Interface
Hi, I will Share How to use it!<br>
this code is belongs to .so file, so You can use as you Like<br>
```capi
# Load a module
capi.loadmod("hello_mod.so");

# Include another script
incl <shared.capi>;

# Set a header
header: "X-Test: Hello";

# Override status code
http.code: 403;

# Execute command
capi.exec: "/bin/echo Hello from Shell";

# Define and call
int hi = "capi rocks!";
function testfunc():
echo: hi;
funclose;
testfunc();

# Close early
capi.forceclose();
```

# How To Compile
*How To Compile*
First Type<br>
```bash
git clone https://github.com/mereka-vpu/libcapi-dev
```
Then<br>
```bash
gcc -fPIC -shared -o libcapi.so libcapi.c -ldl
```
After that you can use as you like!<br>
start contributing by edit the file!
