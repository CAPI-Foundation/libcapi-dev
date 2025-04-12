# Create Application Programmable Interface
Hi, I will Share How to use it!<br>
this code is belongs to .so file, so You can use as you Like<br>
```capi

# Include another script
incl <shared.capi>;

# Execute command
capi.exec: "/bin/echo Hello from Shell";

# Define and call
int hi= "capi rocks!";
function testfunc():
echo: hi;
funclose;
testfunc();

# Close early
capi.forceclose();
```

You can update your "How To Compile" instructions with the `make install` command like this:


# How To Compile

*How To Compile*

First, type:

```bash
git clone https://github.com/mereka-vpu/libcapi-dev
```

Then, navigate to the directory:

```bash
cd libcapi-dev
```

Next, compile the project by running:

```bash
make
```

Finally, install the shared library to `/usr/local/lib` and update the linker cache:

```bash
sudo make install
```

This will copy the `libcapi.so` file to `/usr/local/lib` and run `ldconfig` to update the dynamic linker cache.

# Simple Version

First Execute this command

```bash
sudo add-apt-repository ppa:neoncorp/libcapi
```

Then after that

```bash
sudo apt install libcapi
```

This provides a clear set of instructions for both compiling and installing the library.
After that you can use as you like!<br>
start contributing by edit the file!
