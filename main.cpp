// This is free and unencumbered software released into the public domain.

// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <fcntl.h>     // For O_* constants.
#include <sys/mman.h>  // shm_open, shm_unlink, mmap etc */
#include <sys/stat.h>  // For mode constants
#include <unistd.h>    // For resizing shared memory segment.

#include <cstring>   // memcpy, memset
#include <iostream>  // C++ I/O
#include <string>    // std::string class because it's easy to use.

// BEFORE YOU BEGIN
// ----------------
//
// Please note that in terms of style, this is not great C++. In a proper
// application you'd probably want to wrap a lot of this code into functions
// and/or classes. For the purposes of teaching, it is easier to read this as
// a single procedure though, which is why I put it all in main.
//
// Hope this helps!

int main(int argc, char* argv[]) {
  // Pass "-c" flag to second call to a.out to read a message before writing.
  const bool read_first = ((argc == 2) && (0 == std::strcmp(argv[1], "-c")));

  // Open the shared memory file. We called the shared memory file "foo.shm",
  // but you can call it whatever you want as long as all processes use the
  // same name.
  //
  // The second parameter tells the function to create the file if it doesn't
  // exist, and to open in read/write mode.
  //
  // The third parameter tells the function th make the file readable and
  // writeable by the current user.
  auto fd = shm_open("foo.shm", O_RDWR | O_CREAT, S_IRWXU);

  if (fd == -1) {
    // If the returned file descriptor is -1, that means the shared memory file
    // couldn't be opened.
    std::cout << "Failed to open shared memory file.\n";
    return 1;
  }

  // Use the `ftruncate()` function to give us 100 bytes of shared memory.
  int truncate_result = ftruncate(fd, 100);

  if (truncate_result != 0) {
    // Couldn't resize the shared memory file. `truncate_result` contains an
    // error code.
    std::cout << "Failed to set shared memory file size.\n";
    close(fd);

    // `shm_unlink()` deletes the shared memory file.
    shm_unlink("foo.shm");
    return 1;
  }

  // Map the shared memory file into our local process' memory.
  // Check the mmap manpage for details (or search `man mmap` on a search
  // engine of your choice). Things to note here:
  //
  // * Use nullptr almost always as the first parameter.
  // * Second parameter is the mapped size. You should probably set it to match
  //   the value set in your call to `ftruncate()` above.
  // * Third parameter sets the access rights on the memory section.
  // * Fifth parameter is the file descriptor created in `shm_open()` above.
  //
  // One thing to note is that `mmap()` will reserve a minimum of 1 memory page,
  // (usually about 4 kibibytes), so bear that in mind if you want guidance on
  // array sizes and stuff.
  auto p = mmap(nullptr, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == nullptr) {
    std::cout << "Failed to map shared memory to local address space\n";
    close(fd);
    shm_unlink("foo.shm");
    return 1;
  }

  // Close the shared memory file. Once the shared memory is mapped, you don't
  // need to keep the file open anymore. Any changes you make to the memory
  // mapped area still be propagated to other processes.
  close(fd);

  // Zero the shared memory section.
  std::memset(p, 0, 100);

  bool skip_write_msg = read_first;
  while (true) {
    std::string str;

    if (skip_write_msg) {
      // If we set the -c flag in the command line, don't write this time, skip
      // straight to the read bit.
      skip_write_msg = false;
    } else {
      // Lets write something into shared memory!
      std::cout << "Send message (type 'exit' to quit): ";
      std::cin >> str;
      std::cin.get();

      if (str == "exit")
        break;

      std::cout << "Sending '" << str << "' (length: " << str.size() << ")\n";
      (void)std::memcpy(p, str.c_str(), str.length() + 1);
    }

    // Lets read something from shared memory. Lets wait until we're ready. No
    // complex stuff here, just write a message with our counterpart process
    std::cout << "Hit return when ready to read...";
    std::cin.get();

    // Read the shared memory data as if it was a string.
    std::cout << "Received: " << static_cast<const char*>(p) << std::endl;
  }

  // Delete the shared memory file, but only if the app was started without
  // command line args. This ensures that only one process will attempt to
  // delete the shared memory file.
  if (!read_first) {
    shm_unlink("foo.shm");
  }

  return 0;
}
