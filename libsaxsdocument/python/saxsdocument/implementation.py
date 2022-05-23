
import saxsdocument.api as api


def read_dat(filename):
  doc = api.read(filename, "atsas-dat-3-column");
  s, I, err = doc.curve(0).data()
  properties = doc.properties()
  return s, I, err, properties

def write_dat(filename, s, I, err, properties):
  doc = api.create()
  doc.add_curve(s, I, err)
  doc.add_properties(properties)
  doc.write(filename, "atsas-dat-3-column")


def read_fir(filename):
  doc = api.read(filename, "atsas-fir-4-column");
  s0, I0, err0 = doc.curve(0).data()
  s1, I1, err1 = doc.curve(1).data()
  properties = doc.properties()
  return s0, I0, err0, I1, properties

def write_fir(filename, s, Iexp, errExp, Ifit, properties):
  doc = api.create()
  doc.add_curve(s, Iexp, errExp)
  doc.add_curve(s, Ifit, [0.0] * len(errExp))
  doc.add_properties(properties)
  doc.write(filename, "atsas-fir-4-column")

