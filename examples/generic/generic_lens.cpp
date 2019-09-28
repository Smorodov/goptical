/*

      This file is part of the <goptical/core library.

      The <goptical/core library is free software; you can redistribute it
      and/or modify it under the terms of the GNU General Public
      License as published by the Free Software Foundation; either
      version 3 of the License, or (at your option) any later version.

      The <goptical/core library is distributed in the hope that it will be
      useful, but WITHOUT ANY WARRANTY; without even the implied
      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
      See the GNU General Public License for more details.

      You should have received a copy of the GNU General Public
      License along with the <goptical/core library; if not, write to the
      Free Software Foundation, Inc., 59 Temple Place, Suite 330,
      Boston, MA 02111-1307 USA

      Copyright (C) 2010-2011 Free Software Foundation, Inc
      Author: Alexandre Becoulet

*/

/* -*- indent-tabs-mode: nil -*- */

#include <fstream>
#include <iostream>

#include <goptical/core/math/Vector>

#include <goptical/core/material/Abbe>

#include <goptical/core/sys/Image>
#include <goptical/core/sys/Lens>
#include <goptical/core/sys/Source>
#include <goptical/core/sys/SourcePoint>
#include <goptical/core/sys/SourceRays>
#include <goptical/core/sys/Stop>
#include <goptical/core/sys/System>

#include <goptical/core/curve/Rotational>
#include <goptical/core/curve/Sphere>
#include <goptical/core/shape/Disk>

#include <goptical/core/trace/Distribution>
#include <goptical/core/trace/Params>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Sequence>
#include <goptical/core/trace/Tracer>

#include <goptical/core/light/SpectralLine>

#include <goptical/core/analysis/RayFan>
#include <goptical/core/analysis/Spot>
#include <goptical/core/data/Plot>

#include <goptical/core/io/RendererSvg>
#include <goptical/core/io/Rgb>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iterator>
#include <string>
#include <vector>
#include <memory>

using namespace goptical;

class AsphericCurve : public curve::Rotational {

public:
  AsphericCurve(double r, double k, double A4, double A6, double A8, double A10,
                double A12 = 0.0, double A14 = 0.0)
      : _r(r), _k(k), _A4(A4), _A6(A6), _A8(A8), _A10(A10), _A12(A12),
        _A14(A14) {}

  double sagitta(double y) const override {
    double yy = y * y;
    double y4 = yy * yy;
    double y6 = y4 * yy;
    double y8 = y6 * yy;
    double y10 = y8 * yy;
    double y12 = y10 * yy;
    double y14 = y12 * yy;
    double rr = _r * _r;
    double z = (yy / _r) / (1.0 + pow(1.0 - _k * (yy / rr), 0.5)) + _A4 * y4 +
               _A6 * y6 + _A8 * y8 + _A10 * y10 + _A12 * y12 + _A14 * y14;
    return z;
  }

private:
  double _r;
  double _k;
  double _A4;
  double _A6;
  double _A8;
  double _A10;
  double _A12;
  double _A14;
};

class DescriptiveData {
public:
  const std::string &get_title() const { return title_; }
  void set_title(const std::string &title) { title_ = title; }

private:
  std::string title_;
};

class Variable {
public:
  Variable(const char *name, const char *value) : name_(name), value_(value) {}
  const std::string name() const { return name_; }
  const std::string value() const { return value_; }

private:
  std::string name_;
  std::string value_;
};

class AsphericalData {

public:
  AsphericalData(int surface_number) : surface_number_(surface_number) {}
  void add_data(double d) { data_.push_back(d); }
  int data_points() const { return data_.size(); }
  double data(int i) const {
    return i >= 0 && i < data_.size() ? data_[i] : 0.0;
  }

private:
  int surface_number_;
  std::vector<double> data_;
};

enum LensSurfaceType { surface, aperture_stop };

class SurfaceBuilder {
public:
  SurfaceBuilder(int id)
      : id_(id), surface_type_(LensSurfaceType::surface), radius_(0),
        thickness_(0), ap_radius_(0), refractive_index_(0), abbe_vd_(0),
        aspherical_data_(nullptr) {}
  LensSurfaceType get_surface_type() const { return surface_type_; }
  void set_surface_type(LensSurfaceType surface_type) {
    surface_type_ = surface_type;
  }
  double get_radius() const { return radius_; }
  void set_radius(double radius) { radius_ = radius; }
  double get_thickness() const { return thickness_; }
  void set_thickness(double thickness) { thickness_ = thickness; }
  double get_ap_radius() const { return ap_radius_; }
  void set_ap_radius(double ap_radius) { ap_radius_ = ap_radius; }
  double get_refractive_index() const { return refractive_index_; }
  void set_refractive_index(double refractive_index) {
    refractive_index_ = refractive_index;
  }
  double get_abbe_vd() const { return abbe_vd_; }
  void set_abbe_vd(double abbe_vd) { abbe_vd_ = abbe_vd; }
  std::shared_ptr<AsphericalData> get_aspherical_data() const { return aspherical_data_; }
  void set_aspherical_data(std::shared_ptr<AsphericalData> aspherical_data) {
    aspherical_data_ = aspherical_data;
  }
  int get_id() const { return id_; }

  // Return < 0 on error
  double add_surface(sys::Lens &lens);

private:
  int id_;
  LensSurfaceType surface_type_;
  double radius_;
  double thickness_;
  double ap_radius_;
  double refractive_index_;
  double abbe_vd_;
  std::shared_ptr<AsphericalData> aspherical_data_;
};

class LensSystem {
public:
  bool parseFile(const std::string &file_name);
  double add_surfaces(sys::Lens &lens) {
    double total = 0.0;
    for (int i = 0; i < surfaces_.size(); i++) {
      double thickness = surfaces_[i].add_surface(lens);
      if (thickness < 0.0)
        return thickness;
      total += thickness;
    }
    printf("Image position is at %f\n", total);
    return total;
  }

  const Variable *find_variable(const char *name) const {
    for (int i = 0; i < variables_.size(); i++) {
      if (strcmp(name, variables_[i].name().c_str()) == 0) {
        return &variables_[i];
      }
    }
    return nullptr;
  }

  double get_image_height() const {
    return get_variable_or_value("Image Height");
  }

  double get_variable_or_value(const char *value) const {
    if (value[0] == 0)
      return 0.0;
    if (isalpha(value[0])) {
      const Variable *var = find_variable(value);
      if (var != nullptr) {
        return strtod(var->value().c_str(), NULL);
      } else {
        fprintf(stderr, "Variable %s was not found\n", value);
        return 0.0;
      }
    } else {
      return strtod(value, NULL);
    }
  }

  SurfaceBuilder *find_surface(int id) {
    for (int i = 0; i < surfaces_.size(); i++) {
      if (surfaces_[i].get_id() == id)
        return &surfaces_[i];
    }
    return nullptr;
  }

private:
  DescriptiveData descriptive_data_;
  std::vector<Variable> variables_;
  std::vector<SurfaceBuilder> surfaces_;
  std::vector<std::shared_ptr<AsphericalData>> aspherical_data_;
};

double SurfaceBuilder::add_surface(sys::Lens &lens) {
  if (surface_type_ == LensSurfaceType::aperture_stop) {
    lens.add_stop(ap_radius_, thickness_);
    return thickness_;
  }
  if (aspherical_data_ == nullptr) {
    if (refractive_index_ != 0.0) {
      if (abbe_vd_ == 0.0) {
        fprintf(stderr, "Abbe vd not specified for surface %d\n", id_);
        return -1.0;
      }
      lens.add_surface(
          radius_, ap_radius_, thickness_,
          ref<material::AbbeVd>::create(refractive_index_, abbe_vd_));
    } else {
      lens.add_surface(radius_, ap_radius_, thickness_);
    }
    return thickness_;
  }
  double k = aspherical_data_->data(1)+1.0;
  double a4 = aspherical_data_->data(2);
  double a6 = aspherical_data_->data(3);
  double a8 = aspherical_data_->data(4);
  double a10 = aspherical_data_->data(5);
  double a12 = aspherical_data_->data(6);
  double a14 = aspherical_data_->data(7);

  if (refractive_index_ > 0.0) {
    lens.add_surface(
        ref<AsphericCurve>::create(radius_, k, a4, a6, a8, a10),
        ref<shape::Disk>::create(ap_radius_), thickness_,
        ref<material::AbbeVd>::create(refractive_index_, abbe_vd_));
  } else {
    lens.add_surface(ref<AsphericCurve>::create(radius_, k, a4, a6, a8, a10),
                     ref<shape::Disk>::create(ap_radius_), thickness_);
  }
  return thickness_;
}

// Sizeof buf must be == sizeof input
static bool parse_delimited(char *input_start, size_t input_size,
                            std::vector<const char *> &out_tokens, char *buf,
                            const char *delimiters) noexcept {
  out_tokens.clear();

  if (input_size == 0) {
    return true;
  }
  const char *input_ptr = input_start;
  const char *input_end = input_start + input_size;
  char *wordp = buf;

  while (*input_ptr && input_ptr != input_end) {
    char *word = wordp;
    *wordp = 0;

    bool inquote = false;
    while (*input_ptr && input_ptr != input_end) {
      if (word == wordp) {
        // we are at the beginning for a word, so look
        // for potential quote
        if (*input_ptr == '"' && !inquote) {
          // We are in a quoted word
          inquote = true;
          input_ptr++;
          continue;
        }
      }
      if (inquote) {
        // We are in a quoted word
        if (*input_ptr == '"') {
          // Check if it is an escape - i.e.
          // double quote
          if (input_ptr + 1 < input_end && *(input_ptr + 1) == '"') {
            // escape so we add the quote
            // character
            *wordp++ = '"';
            input_ptr += 2;
            continue;
          } else {
            // not escape so the quoted word
            // ends here
            inquote = false;
            *wordp++ = 0;
            input_ptr++;
            if (input_ptr < input_end &&
                (*input_ptr == ',' || *input_ptr == '\t' ||
                 (delimiters && strchr(delimiters, *input_ptr)))) {
              // Skip delimiter
              // following quote
              input_ptr++;
            }
            break;
          }
        } else {
          // still in quoted word
          *wordp++ = *input_ptr++;
          continue;
        }
      } else {
        // Not in quoted word
        if (*input_ptr == ',' || *input_ptr == '\t' ||
            (delimiters && strchr(delimiters, *input_ptr))) {
          // word ends due to delimiter
          *wordp++ = 0;
          input_ptr++;
          break;
        } else if (*input_ptr == '\r' || *input_ptr == '\n') {
          // skip line feed or CRLF
          *wordp++ = 0;
          if (*input_ptr == '\r' && input_ptr + 1 < input_end &&
              *(input_ptr + 1) == '\n') {
            input_ptr++;
          }
          input_ptr++;
          break;
        } else {
          *wordp++ = *input_ptr++;
        }
      }
    }
    out_tokens.push_back(word);
  }
  return true;
}

struct SectionMapping {
  const char *name;
  int section;
};

SectionMapping g_SectionMappings[] = {{"[descriptive data]", 1},
                                      {"[variable distances]", 2},
                                      {"[lens data]", 3},
                                      {"[aspherical data]", 4},
                                      {NULL, 0}};

static int find_section(const char *name) {
  int section = 0;
  for (int i = 0; i < sizeof g_SectionMappings / sizeof(SectionMapping); i++) {
    if (g_SectionMappings[i].name == NULL) {
      section = g_SectionMappings[i].section;
      break;
    } else if (strcmp(g_SectionMappings[i].name, name) == 0) {
      section = g_SectionMappings[i].section;
      break;
    }
  }
  return section;
}

bool LensSystem::parseFile(const std::string &file_name) {

  FILE *fp = fopen(file_name.c_str(), "r");
  if (fp == NULL) {
    fprintf(stderr, "Unable to open file %s: %s\n", file_name.c_str(),
            strerror(errno));
    return false;
  }

  char line[256];                  // input line
  char buf[256];                   // for tokenizing
  std::vector<const char *> words; // tokenized words
  int current_section = 0;         // Current section

  while (fgets(line, sizeof line, fp) != NULL) {

    if (!parse_delimited(line, sizeof line, words, buf, "\t\n")) {
      continue;
    }
    if (words.size() == 0) {
      continue;
    }
    if (words[0][0] == '#') {
      // comment
      continue;
    }
    if (words[0][0] == '[') {
      // section name
      current_section = find_section(words[0]);
      continue;
    }

    switch (current_section) {
    case 1:
      if (words.size() >= 2 && strcmp(words[0], "title") == 0) {
        descriptive_data_.set_title(words[1]);
      }
      break;
    case 2:
      if (words.size() == 2) {
        variables_.push_back(Variable(words[0], words[1]));
      }
      break;
    case 3: {
      int id = atoi(words[0]);
      double radius = 0;
      double thickness = 0;
      double ap_radius = 0;
      double refractive_index = 0;
      double abbe_vd = 0;
      LensSurfaceType type = LensSurfaceType::surface;
      if (words.size() >= 2 && strcmp(words[1], "AS") == 0) {
        type = LensSurfaceType::aperture_stop;
      } else {
        radius = get_variable_or_value(words[1]);
      }
      if (words.size() >= 3 && strlen(words[2]) > 0) {
        thickness = get_variable_or_value(words[2]);
      }
      if (words.size() >= 4 && strlen(words[3]) > 0) {
        refractive_index = get_variable_or_value(words[3]);
      }
      if (words.size() >= 5 && strlen(words[4]) > 0) {
        ap_radius = get_variable_or_value(words[4]) / 2.0;
      }
      if (words.size() >= 6 && strlen(words[5]) > 0) {
        abbe_vd = get_variable_or_value(words[5]);
      }
      SurfaceBuilder surface_data(id);
      surface_data.set_abbe_vd(abbe_vd);
      surface_data.set_ap_radius(ap_radius);
      surface_data.set_refractive_index(refractive_index);
      surface_data.set_radius(radius);
      surface_data.set_thickness(thickness);
      surface_data.set_surface_type(type);
      surfaces_.push_back(surface_data);
    } break;
    case 4: {
      int id = atoi(words[0]);
      auto aspherical_data = std::make_shared<AsphericalData>(id);
      for (int i = 1; i < words.size(); i++) {
        aspherical_data->add_data(strtod(words[i], NULL));
      }
      aspherical_data_.push_back(aspherical_data);
      SurfaceBuilder *surface_builder = find_surface(id);
      if (surface_builder == nullptr) {
        fprintf(stderr, "Ignoring aspherical data as no surface numbered %d\n",
                id);
      } else {
        surface_builder->set_aspherical_data(
            aspherical_data_.back());
      }
    } break;
    default:
      break;
    }
  }

  fclose(fp);
  return true;
}

int main(int argc, const char *argv[]) {
  //**********************************************************************
  // Optical system definition

  if (argc != 2) {
    fprintf(stderr, "Please supply a data file\n");
    exit(1);
  }

  LensSystem lens_data;
  if (!lens_data.parseFile(argv[1])) {
    exit(1);
  }

  sys::System sys;

  /* anchor lens */
  sys::Lens lens(math::Vector3(0, 0, 0));
  double image_pos = lens_data.add_surfaces(lens);
  sys.add(lens);
  /* anchor end */
  auto &s1 = lens.get_surface(0);

  sys::Image image(math::Vector3(0, 0, image_pos),
                   lens_data.get_image_height());
  sys.add(image);

  sys.set_entrance_pupil(s1);

  /* anchor sources */
  //  sys::SourceRays  source_rays(math::Vector3(0, 27.5, -100000));
  //
  //  sys::SourcePoint source_point(sys::SourceAtFiniteDistance,
  //                                math::Vector3(0, 27.5, -100000));

  //  sys::SourcePoint source_point(sys::SourceAtInfinity,
  //                                math::Vector3(0, 0, 1));

  sys::SourcePoint source_point(sys::SourceAtInfinity, math::Vector3(0, 0, 1));

  // add sources to system
  //  sys.add(source_rays);
  sys.add(source_point);

  // configure sources
  //  source_rays.add_chief_rays(sys);
  //  source_rays.add_marginal_rays(sys, 14);

  source_point.clear_spectrum();
  source_point.add_spectral_line(light::SpectralLine::d);
  /* anchor end */

  /* anchor seq */
  trace::Sequence seq(sys);

  //  sys.get_tracer_params().set_sequential_mode(seq);
  std::cout << "system:" << std::endl << sys;
  std::cout << "sequence:" << std::endl << seq;
  /* anchor end */

  //**********************************************************************
  // Drawing rays and layout

  {
    /* anchor layout */
    io::RendererSvg renderer("layout.svg", 800, 400);

#if 1
    // draw 2d system layout
    sys.draw_2d_fit(renderer);
    sys.draw_2d(renderer);
#else
    // draw 2d layout of lens only
    lens.draw_2d_fit(renderer);
    lens.draw_2d(renderer);
#endif

    trace::Tracer tracer(sys);

#if 0
    // trace and draw rays from rays source
    sys.enable_single<sys::Source>(source_rays);
    tracer.get_trace_result().set_generated_save_state(source_rays);

    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
    /* anchor end */
#else
    // trace and draw rays from source
    tracer.get_params().set_default_distribution(
        trace::Distribution(trace::MeridionalDist, 10));
    tracer.get_trace_result().set_generated_save_state(source_point);
    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
#endif
  }

  {
    /* anchor spot */
    sys.enable_single<sys::Source>(source_point);

    sys.get_tracer_params().set_default_distribution(
        trace::Distribution(trace::HexaPolarDist, 20));

    analysis::Spot spot(sys);

    /* anchor end */
    {
      /* anchor spot */
      io::RendererSvg renderer("spot.svg", 300, 300, io::rgb_black);

      spot.draw_diagram(renderer);
      /* anchor end */
    }

    {
      /* anchor spot_plot */
      io::RendererSvg renderer("spot_intensity.svg", 640, 480);

      ref<data::Plot> plot = spot.get_encircled_intensity_plot(50);

      plot->draw(renderer);
      /* anchor end */
    }
  }

  {
    /* anchor opd_fan */
    sys.enable_single<sys::Source>(source_point);

    analysis::RayFan fan(sys);

    /* anchor end */
    {
      /* anchor opd_fan */
      io::RendererSvg renderer("opd_fan.svg", 640, 480);

      ref<data::Plot> fan_plot = fan.get_plot(
          analysis::RayFan::EntranceHeight, analysis::RayFan::OpticalPathDiff);

      fan_plot->draw(renderer);

      /* anchor end */
    }

    {
      /* anchor transverse_fan */
      io::RendererSvg renderer("transverse_fan.svg", 640, 480);

      ref<data::Plot> fan_plot =
          fan.get_plot(analysis::RayFan::EntranceHeight,
                       analysis::RayFan::TransverseDistance);

      fan_plot->draw(renderer);

      /* anchor end */
    }

    {
      /* anchor longitudinal_fan */
      io::RendererSvg renderer("longitudinal_fan.svg", 640, 480);

      ref<data::Plot> fan_plot =
          fan.get_plot(analysis::RayFan::EntranceHeight,
                       analysis::RayFan::LongitudinalDistance);

      fan_plot->draw(renderer);

      /* anchor end */
    }
  }
  return 0;
}
