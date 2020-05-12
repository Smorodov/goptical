#ifndef GOPTICAL_IMPORT_BCLAFF_HPP
#define GOPTICAL_IMPORT_BCLAFF_HPP

#include <goptical/core/sys/System>
#include <goptical/core/sys/Image>

#include <memory>

namespace goptical {
  namespace io {
    class LensSpecifications;
    class BClaffLensImporter
    {
    public:
      BClaffLensImporter();
      virtual ~BClaffLensImporter();
      BClaffLensImporter (const BClaffLensImporter &) = delete;
      BClaffLensImporter &operator= (const BClaffLensImporter &) = delete;
      bool parseFile (const std::string &file_name);
      std::shared_ptr<sys::System> buildSystem(unsigned scenario = 0);
      double getAngleOfViewInRadians (unsigned scenario = 0);
      std::shared_ptr<sys::Image> get_image() const { return image_; }
      std::shared_ptr<sys::System> get_system() const { return sys_; }
    private:
      std::unique_ptr<LensSpecifications> specs_;
      std::shared_ptr<sys::Image> image_;
      std::shared_ptr<sys::System> sys_;
    };

  } // namespace io

} // namespace goptical

#endif // GOPTICAL_IMPORT_BCLAFF_HPP
