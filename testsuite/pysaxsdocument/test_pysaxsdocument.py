import sys
import os
import unittest

import saxsdocument


class TestSAXSDocument(unittest.TestCase):

    def test_read(self):
        saxsdocument.read("bsa.dat")


if __name__ == "__main__":
    unittest.main()
