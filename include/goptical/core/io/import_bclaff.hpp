#ifndef GOPTICAL_IMPORT_BCLAFF_HPP
#define GOPTICAL_IMPORT_BCLAFF_HPP

#include <goptical/core/sys/System>

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
      bool parseFile (const std::string &file_name, sys::System &sys,
			unsigned scenario = 0);
      double getAngleOfViewInRadians (unsigned scenario = 0);

    private:
      std::unique_ptr<LensSpecifications> specs_;
    };

  } // namespace io

} // namespace goptical

#endif // GOPTICAL_IMPORT_BCLAFF_HPP
