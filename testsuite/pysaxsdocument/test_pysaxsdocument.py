import sys
import os
import unittest

import saxsdocument


class TestSAXSDocument(unittest.TestCase):

    def setUp(self):
        self.doc = saxsdocument.read("bsa.dat")

    def test_curves(self):
        curves = self.doc.curve
        self.assertIsInstance(curves, list)
        self.assertEqual(len(curves), 1)
        self.assertIsInstance(curves[0], list)
        self.assertEqual(len(curves[0]), 2096)
        self.assertTupleEqual(curves[0][0],
                              (0.741270E-01, 0.260465E+05, 0.321129E+02))

    def test_properties(self):
        properties = self.doc.property
        self.assertIsInstance(properties, dict)
        self.assertEqual(properties["Sample code"], "BSA")


if __name__ == "__main__":
    unittest.main()
