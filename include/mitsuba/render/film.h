#pragma once

#include <mitsuba/mitsuba.h>
#include <mitsuba/core/logger.h>
#include <mitsuba/core/object.h>
#include <mitsuba/core/rfilter.h>
#include <mitsuba/core/vector.h>
#include <mitsuba/render/sampler.h>
#include <mitsuba/render/fwd.h>

NAMESPACE_BEGIN(mitsuba)

/** \brief Abstract film base class - used to store samples
 * generated by \ref Integrator implementations.
 *
 * To avoid lock-related bottlenecks when rendering with many cores,
 * rendering threads first store results in an "image block", which
 * is then committed to the film using the \ref put() method.
 */
template <typename Float, typename Spectrum>
class MTS_EXPORT_RENDER Film : public Object {
public:
    MTS_IMPORT_TYPES(ImageBlock, ReconstructionFilter)

    /// Configure the film for rendering a specified set of channels
    virtual void prepare(const std::vector<std::string> &channels) = 0;

    /// Merge an image block into the film. This methods should be thread-safe.
    virtual void put(const ImageBlock *block) = 0;

    /// Develop the film and write the result to the previously specified filename
    virtual void develop() = 0;

    /// Overwrite the weight channel to the given value
    virtual void reweight(double weight) = 0;

    /**
     * \brief Develop the contents of a subregion of the film and store
     * it inside the given bitmap
     *
     * This may fail when the film does not have an explicit representation
     * of the bitmap in question (e.g. when it is writing to a tiled EXR image)
     *
     * \return \c true upon success
     */
    virtual bool develop(
        const ScalarPoint2i  &offset,
        const ScalarVector2i &size,
        const ScalarPoint2i  &target_offset,
        Bitmap *target) const = 0;

    /// Return a bitmap object storing the developed contents of the film
    virtual ref<Bitmap> bitmap(bool raw = false) = 0;

    /// Set the target filename (with or without extension)
    virtual void set_destination_file(const fs::path &filename) = 0;

    /// Does the destination file already exist?
    virtual bool destination_exists(const fs::path &basename) const = 0;

    /**
     * Should regions slightly outside the image plane be sampled to improve
     * the quality of the reconstruction at the edges? This only makes
     * sense when reconstruction filters other than the box filter are used.
     */
    bool has_high_quality_edges() const { return m_high_quality_edges; }

    // =============================================================
    //! @{ \name Accessor functions
    // =============================================================

    /// Ignoring the crop window, return the resolution of the underlying sensor
    const ScalarVector2i &size() const { return m_size; }

    /// Return the size of the crop window
    const ScalarVector2i &crop_size() const { return m_crop_size; }

    /// Return the offset of the crop window
    const ScalarPoint2i &crop_offset() const { return m_crop_offset; }

    /// Set the size and offset of the crop window.
    void set_crop_window(const ScalarPoint2i &crop_offset,
                         const ScalarVector2i &crop_size);

    /// Return the image reconstruction filter (const version)
    const ReconstructionFilter *reconstruction_filter() const {
        return m_filter.get();
    }

    //! @}
    // =============================================================

    virtual std::string to_string() const override;

    MTS_DECLARE_CLASS()
protected:
    /// Create a film
    Film(const Properties &props);

    /// Virtual destructor
    virtual ~Film();

protected:
    ScalarVector2i m_size;
    ScalarVector2i m_crop_size;
    ScalarPoint2i m_crop_offset;
    bool m_high_quality_edges;
    ref<ReconstructionFilter> m_filter;
};

MTS_EXTERN_CLASS_RENDER(Film)
NAMESPACE_END(mitsuba)
