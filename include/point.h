#ifndef datapoint_h
#define datapoint_h

struct DataPoint {
  unsigned long time = 0;
  float altitude = 0.0;
  float acceleration_x = 0.0;
  float acceleration_y = 0.0;
  float acceleration_z = 0.0;
  float rotation_x = 0.0;
  float rotation_y = 0.0;
  float rotation_z = 0.0;
};

#endif