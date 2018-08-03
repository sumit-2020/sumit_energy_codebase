#!/usr/bin/python

from functions import *

def missing_tests(dimm):
  singlebankpairs = [
    ("singlebankdoublecol","d00"), ("singlebankdoublecol","d01"), ("singlebankdoublecol","d02"),
    ("singlebankdoublecol","d03"), ("singlebankdoublecol","d04"), ("singlebankdoublecol","d05"),
    ("singlebankdoublecol","d06"), ("singlebankdoublecol","d07")
  ]
  
  doublebankpairs = [
    ("doublebankdoublecol","d08"), ("doublebankdoublecol","d10"),("doublebankdoublecol","d11"),
    ("doublebankdoublecol","b51"),("doublebankdoublecol","b73"),("doublebankdoublecol","b34")
  ]

  bicipairs =[
    ("bici","d08"), ("bici","d10"), ("bici","d11"),
    ("bici","b51"), ("bici","b73"), ("bici","b34")
  ]

  tests = []

  for (test, addr_set) in bicipairs:
    if dimm in ["micronb59", "micronb60"]:
      # 24 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd0, "patt2": 0xd0 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x13, "patt2": 0x13 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0d, "patt2": 0x0d })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x31, "patt2": 0x31 })

      # 48 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x3f, "patt2": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf3, "patt2": 0xf3 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xcf, "patt2": 0xcf })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfc, "patt2": 0xfc })

      # 64 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })

      # 100% (32)
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })

      # 75% (24)
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x5f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x5f })

    if dimm in ["micronb59", "micronb60", "hynixp80"]:
      # 56 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7f, "patt2": 0x7f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf7, "patt2": 0xf7 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xef, "patt2": 0xef })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfe, "patt2": 0xfe })

  for (test, addr_set) in singlebankpairs:

    if dimm in ["samsungo18", "micronb60"]:
      # 56 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7f, "patt2": 0x7f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf7, "patt2": 0xf7 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xef, "patt2": 0xef })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfe, "patt2": 0xfe })

    if dimm in ["hynixp79"]:
      # 100% (32)
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })

      # 75% (24)
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x5f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x5f })

  for (test, addr_set) in doublebankpairs:
      # 24 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd0, "patt2": 0xd0 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x13, "patt2": 0x13 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0d, "patt2": 0x0d })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x31, "patt2": 0x31 })

      # 48 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x3f, "patt2": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf3, "patt2": 0xf3 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xcf, "patt2": 0xcf })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfc, "patt2": 0xfc })

      # 56 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7f, "patt2": 0x7f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf7, "patt2": 0xf7 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xef, "patt2": 0xef })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfe, "patt2": 0xfe })

      # 64 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })

      # 100% (32)
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })

      # 75% (24)
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x5f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x5f })

  

  return tests

def tests_for_donghyuk():
  tests = []
  test_addr_set_pairs = [
    ("singlebankdoublecol","d00"), ("singlebankdoublecol","d01"), ("singlebankdoublecol","d02"),
    ("singlebankdoublecol","d03"), ("singlebankdoublecol","d04"), ("singlebankdoublecol","d05"),
    ("singlebankdoublecol","d06"), ("singlebankdoublecol","d07"),
    # ("doublebankdoublecol","d08"), ("doublebankdoublecol","d10"),("doublebankdoublecol","d11"),
    # ("doublebankdoublecol","b51"),("doublebankdoublecol","b73"),("doublebankdoublecol","b34")
    # ("bici","d08"), ("bici","d10"), ("bici","d11"),
    # ("bici","b51"), ("bici","b73"), ("bici","b34")
  ]

  for pair in test_addr_set_pairs:
      test = pair[0]
      addr_set = pair[1]
      # Baseline Tests
      # ## NumberOf1s: 2
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x00 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x01, "patt2": 0x01 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x02, "patt2": 0x02 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x04, "patt2": 0x04 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x08, "patt2": 0x08 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x10, "patt2": 0x10 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x20, "patt2": 0x20 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x40, "patt2": 0x40 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x80, "patt2": 0x80 })

      # ## NumberOf1s: 4
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x03, "patt2": 0x03 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x05, "patt2": 0x05 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0a, "patt2": 0x0a })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0c, "patt2": 0x0c })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x30, "patt2": 0x30 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x50, "patt2": 0x50 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xa0, "patt2": 0xa0 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xc0, "patt2": 0xc0 })

      # ## NumberOf1s: 8
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5c, "patt2": 0x5c })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x71, "patt2": 0x71 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x27, "patt2": 0x27 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd4, "patt2": 0xd4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x6c, "patt2": 0x6c })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xb3, "patt2": 0xb3 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x9a, "patt2": 0x9a })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x2b, "patt2": 0x2b })

      # ## NumberOf1s: 10
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x1f, "patt2": 0x1f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7a, "patt2": 0x7a })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x3e, "patt2": 0x3e })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf4, "patt2": 0xf4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x67, "patt2": 0x67 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xcd, "patt2": 0xcd })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xb5, "patt2": 0xb5 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xce, "patt2": 0xce })

      # ## 25% Toggle Tests
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5c, "patt2": 0x6c })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x6c, "patt2": 0x5c })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xb1, "patt2": 0x71 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x71, "patt2": 0xb1 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x27, "patt2": 0x2b })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x2b, "patt2": 0x27 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd4, "patt2": 0xe4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xe4, "patt2": 0xd4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x1f, "patt2": 0x2f })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x2f, "patt2": 0x1f })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x3e, "patt2": 0x5e })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5e, "patt2": 0x3e })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7a, "patt2": 0x7c })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7c, "patt2": 0x7a })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf4, "patt2": 0xf8 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf8, "patt2": 0xf4 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x03 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x05 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xa0 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xc0 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x03, "patt2": 0x00 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x05, "patt2": 0x0a })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xa0, "patt2": 0x00 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xc0, "patt2": 0x00 })

      # ## 50% Toggle Tests
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5c, "patt2": 0x66 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x5c, "patt1": 0x66 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x71, "patt2": 0xb2 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x71, "patt1": 0xb2 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x27, "patt2": 0x1b })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x27, "patt1": 0x1b })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd4, "patt2": 0xe8 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xd4, "patt1": 0xe8 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x1f, "patt2": 0x67 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x1f, "patt1": 0x67 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xce, "patt2": 0x3e })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xce, "patt1": 0x3e })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7a, "patt2": 0x75 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x7a, "patt1": 0x75 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf4, "patt2": 0x79 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xf4, "patt1": 0x79 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x0f })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x0f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x17 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x17 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x3c })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x3c })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xaa })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xaa })

      # ## 75% Toggle Tests
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5c, "patt2": 0x63 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x5c, "patt1": 0x63 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xa6, "patt2": 0x71 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xa6, "patt1": 0x71 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x27, "patt2": 0x9a })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x27, "patt1": 0x9a })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xaa, "patt2": 0xd4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xaa, "patt1": 0xd4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x1f, "patt2": 0xe3 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x1f, "patt1": 0xe3 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xcd, "patt2": 0x3e })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xcd, "patt1": 0x3e })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7a, "patt2": 0xb5 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x7a, "patt1": 0xb5 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf4, "patt2": 0x3b })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xf4, "patt1": 0x3b })

      # ## 100% Toggle Test
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5c, "patt2": 0xa3 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x5c, "patt1": 0xa3 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x8e, "patt2": 0x71 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x8e, "patt1": 0x71 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x27, "patt2": 0xd8 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x27, "patt1": 0xd8 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x2b, "patt2": 0xd4 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x2b, "patt1": 0xd4 })

  return tests

def missing_patterns(dimm) :
  test_addr_set_pairs = [
    ("singlebankdoublecol","d00"), ("singlebankdoublecol","d01"), ("singlebankdoublecol","d02"),
    ("singlebankdoublecol","d03"), ("singlebankdoublecol","d04"), ("singlebankdoublecol","d05"),
    ("singlebankdoublecol","d06"), ("singlebankdoublecol","d07")
    # ("bici","d08"), ("bici","d10"), ("bici","d11"),
    # ("bici","b51"), ("bici","b73"), ("bici","b34")
  ]
  
  tests = []
  
  for pair in test_addr_set_pairs:
      test = pair[0]
      addr_set = pair[1]
      
      # 0

      # 6 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd0, "patt2": 0xd0 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x13, "patt2": 0x13 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0d, "patt2": 0x0d })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x31, "patt2": 0x31 })

      # 12 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x3f, "patt2": 0x3f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf3, "patt2": 0xf3 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xcf, "patt2": 0xcf })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfc, "patt2": 0xfc })

      # 14 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7f, "patt2": 0x7f })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf7, "patt2": 0xf7 })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xef, "patt2": 0xef })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfe, "patt2": 0xfe })

      # 16 ones
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
      #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })

      # # 75
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x3f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x3f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x5f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x5f })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x77 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0x77 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xaf })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xaf })

      # # 100
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xff })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x00, "patt1": 0xff })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xc0, "patt2": 0x3f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0xc0, "patt1": 0x3f })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x77, "patt2": 0x88 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt2": 0x77, "patt1": 0x88 })

      # # 12.5
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x01 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x02 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x04 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x08 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x20 })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x40 })

      # # 37.5
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xd0 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x0d })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0d, "patt2": 0x00 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd0, "patt2": 0x00 })

      # # 62.5
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x2f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xf2 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x2f, "patt2": 0x00 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf2, "patt2": 0x00 })

      # # 87.5
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x7f })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xdf })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xf7 })
      # tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xfd })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xbf })
      # #tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xfe })

  return tests

def main():
  STD_DEV_THRESH = 1.5 # 1.5mV = 30mA
  print "starting"
  args = parse_arguments(True)
  args.log = False
  # tests = missing_patterns(args.dimm)
  # tests = tests + tests_for_donghyuk()
  tests = missing_tests(args.dimm)
  print "I have " + str(len(tests)) + " tests to run! Yeayyy !!!"
  clean_start()

  data_dict = {
      "test_id"  :[],
      "test_n"   :[],
      "addr_set" :[],
      "patt1"    :[],
      "patt2"    :[],
      "raw_data" :[],
      "min"      :[],
      "max"      :[],
      "ave"      :[],
      "stdev"    :[],
      "row"      :[]
  }
  trashed_dict = {
      "test_id"  :[],
      "test_n"   :[],
      "addr_set" :[],
      "patt1"    :[],
      "patt2"    :[],
      "raw_data" :[],
      "min"      :[],
      "max"      :[],
      "ave"      :[],
      "stdev"    :[],
      "row"      :[]
  }

  meter = init_meter_connection()
  dataframe = pd.DataFrame(columns=data_dict.keys())
  trashframe= pd.DataFrame(columns=trashed_dict.keys())
  output_csv = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
  trash_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_trash.csv"
  print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv
  tweet("#"+str(args.dimm)+" starting #doublecoltest")
  for test_id, test in enumerate(tests) :
    progress = float(test_id) / float(len(tests)) * 100.0
    print "["+str_timestamp()+"] Progress: " + str(progress) + " % "

    averages = []
    garbages = []
    attempt  = 0
    remove_max = True
    while len(averages) < args.iter and attempt <= args.iter * 3:
      attempt += 1
      # Measure a new set of values
      run_binary(args.dimm,test["test_n"],test["addr_set"],args.row,test["patt1"],test["patt2"],attempt)
      sleep(0.01)
      data_arr = get_measurements(meter)
      run_binary(args.dimm,"stoploop")
      ave_val = stats.mean(data_arr)
      averages.append(ave_val)
      print "Vave: " + str(ave_val) + " ",

      # Calculate the variation
      #variation = max(averages) - min(averages)
      std_dev = 0
      if len(averages) > 1 :
        std_dev = stats.stdev(averages)
        print "stdev: "+str(std_dev)+" ",

      while ((std_dev > STD_DEV_THRESH) and (len(averages) > 1)):
        if remove_max :
          max_ave = max(averages)
          garbages.append(max_ave)
          averages.remove(max_ave)
          remove_max = False
          print "rm_max: " + str(max_ave),
        else :
          min_ave = min(averages)
          garbages.append(min_ave)
          averages.remove(min_ave)
          remove_max = True
          print "rm_min: " + str(min_ave),

        if len(averages) > 1:
          std_dev = stats.stdev(averages)
        #variation = max(averages) - min(averages)

      print ""
    data_dict = append_data(data_dict, test_id, test, averages, args.row)
    dataframe = dataframe.from_dict(data_dict)
    dataframe.to_csv(output_csv)
    if len(garbages) > 0 :
      trashed_dict = append_data(trashed_dict, test_id, test, garbages, args.row)
      trashframe = trashframe.from_dict(trashed_dict)
      trashframe.to_csv(trash_csv)
  tweet("#"+str(args.dimm)+" is done!")

def run_binary(dimm,test,addr_set="r01",row=17868,patt1=0,patt2=0, attempt=0):
  command  = "./bin/toggletest " + dimm
  command += " " + test
  command += " " + addr_set
  command += " " + str(row)
  command += " " + str(patt1)
  command += " " + str(patt2)
  # command += " " + str(patt3)
  # command += " " + str(1)

  if test != "stoploop":
    print "["+str_timestamp()+"] " + command + " (" + str(attempt).zfill(2) +") " ,

  command += "  > ./log_out/" + "_" + dimm
  command += "_" + test
  command += "_" + addr_set
  command += "_" + str(row)
  command += "_" + str(patt1)
  command += "_" + str(patt2)
  command += ".log"

  sp.call(command, shell=True)

def parse_arguments(print_flag = False):
  parser = argparse.ArgumentParser()
  parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
       help='DIMM Name')
  parser.add_argument('iter', metavar='iter', type=int, default=10,
       help='# of Iterations')
  parser.add_argument('row', metavar='row', type=int, default=17868,
       help='Row number')
  parser.add_argument('--nocam', action='store_true', default=False,
       help='No Camera in the measurement part')
  parser.add_argument('--qt', metavar='qt', type=float, default=0.6,
       help='Quality Threshold of Vision: [# of Samples] / [# of Reads]')
  parser.add_argument('--nos', metavar='nos', type=int, default=30,
       help='# of Samples per Read')

  args = parser.parse_args()

  if (print_flag) :
    print "DIMM Name         : " + args.dimm
    print "# of Iterations   : " + str(args.iter)
    print "Row number        : " + str(args.row)
    print "Quality           : " + str(args.qt)
    print "Number of Samples : " + str(args.nos)

  return args

def append_data(data_dict, test_id, test, data_arr, row):
  raw_data = ','.join(map(str, data_arr))
  data_dict["test_id"].append(test_id)
  data_dict["test_n"].append(test["test_n"])
  data_dict["addr_set"].append(test["addr_set"])
  data_dict["patt1"].append(test["patt1"])
  data_dict["patt2"].append(test["patt2"])
  data_dict["raw_data"].append(raw_data)
  data_dict["min"].append(min(data_arr))
  data_dict["max"].append(max(data_arr))
  data_dict["ave"].append(stats.mean(data_arr))
  data_dict["stdev"].append(stats.pstdev(data_arr))
  data_dict["row"].append(row)
  return data_dict

main()
