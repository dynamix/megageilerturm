require 'json'
require 'matrix'

include Math

def rz(r)
  Matrix[
    [cos(r), - sin(r), 0],
    [sin(r),   cos(r), 0],
    [0     ,        0, 1],
  ]
end

def rx(r)
  Matrix[
    [1,0,0       ],
    [0,cos(r), - sin(r)],
    [0,sin(r),   cos(r)],
  ]
end

def ry(r)
  Matrix[
    [cos(r), 0,sin(r)],
    [0,1,0],
    [-sin(r), 0, cos(r)]
  ]
end


strip = Array.new 12 do [] end

(0..29).each do |a|
  # start from bottom
  a = 29 - a
  r =  a * (180/29.0) * PI / 180.0
  v = [sin(r), cos(r), 0]
  v[0] += a < 30 ? 0.15 : -0.3
  (0..11).each do |segment|
    v2 = ry(30*segment * PI / 180.0) * Matrix.column_vector(v)
    strip[segment] << v2.column(0).to_a
  end
end

result = (0..5).map do |i|
  (strip[i*2] + strip[i*2+1].reverse()).map do |x|
    {point:x}
  end
end.flatten

#IO.write('test.json', result.to_json)
puts result.to_json