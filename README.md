### Aroww DB

Inspired by "Designing Data-Intensive Applications" book and
"What I cannot create I do not understand" quote by R. Feynman.

At a time of writing, simple key-value database, 
created for learning purposes only.



**Stuff to worry about:**
- [x] ~~Different protocols support~~ Dont think need it anyway soon
- [ ] Authentication of connection
- [ ] Encryption of data send (openssl should be fine)
- [ ] Sync and async write
- [ ] Perhaps some simple benchmarking
  ([LevelDB bench](http://www.lmdb.tech/bench/microbench/benchmark.html)
  and [Facebook db_bench](https://github.com/facebook/rocksdb/wiki/Benchmarking-tools)
  looks promising)
- [ ] Still no namespace for some reason, need to add


**Future plans on things to add:**
- [x] Static code analyzers [clang-tidy, cppcheck, clazy]
- [x] Sourse code formatter [clang-format]
- [ ] Documentation and docs generation [doxygen]
- [x] ~~Package managers? [Conan, vcpkg]~~ Okay, I see how bad things are with packaging in C++. 
  Gonna leave this for better times.
- [x] See if `fmt` lib is useful and add if needed
- [x] Logging of event to gather metric [`spdlog` or what?] 
- [x] Implementing some basic tests
- [x] Some way of writing unit tests