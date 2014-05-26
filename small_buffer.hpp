#ifndef SMALL_BUFFER_INCLUDED__
#define SMALL_BUFFER_INCLUDED__

#include <array>
#include <memory>


namespace detail {

    template <typename T, std::size_t Bytes>
    union small_buffer_storage
    {
        static_assert(
            sizeof(std::unique_ptr<T>) <= Bytes,
            "small_buffer_storage size template parameter Bytes must be at least "
            "sizeof(std::unique_ptr<T>)"
        );

        small_buffer_storage () {}
        ~small_buffer_storage () {}

        const T & cast () const
        { return *static_cast<const T *>(static_cast<const void *>(buffer_.data())); }

        T & cast ()
        { return *static_cast<T *>(static_cast<void *>(buffer_.data())); }

        std::array<unsigned char, Bytes> buffer_;
        std::unique_ptr<T> pointer_;
    };

}


template <
    typename T,
    std::size_t Bytes = sizeof(std::unique_ptr<T>),
    bool TooBig = Bytes < sizeof(T)
>
class small_buffer;


template <typename T, std::size_t Bytes>
class small_buffer<T, Bytes, true>
{
public:
    small_buffer ()
    { new (&storage_.pointer_) std::unique_ptr<T>(new T()); }

    explicit small_buffer (T value)
    { new (&storage_.pointer_) std::unique_ptr<T>(new T(std::move(value))); }

    small_buffer (const small_buffer & rhs)
    { new (&storage_.pointer_) std::unique_ptr<T>(new T(*rhs.storage_.pointer_)); }

    small_buffer (small_buffer && rhs)
    { new (&storage_.pointer_) std::unique_ptr<T>(std::move(rhs.storage_.pointer_)); }

    small_buffer & operator= (const small_buffer & rhs)
    {
        storage_.pointer_.~unique_ptr();
        new (&storage_.pointer_)
            std::unique_ptr<T>(new T(*rhs.storage_.pointer_));
        return *this;
    }

    small_buffer & operator= (small_buffer && rhs)
    {
        std::swap(storage_.pointer_, rhs.storage_.pointer_);
        return *this;
    }

    ~small_buffer ()
    { storage_.pointer_.~unique_ptr(); }

    T & value ()
    { return operator*(); }

    T & operator* ()
    { return *storage_.pointer_; }

    T * operator-> ()
    { return storage_.pointer_.get(); }

private:
    detail::small_buffer_storage<T, Bytes> storage_;
};

template <typename T, std::size_t Bytes>
class small_buffer<T, Bytes, false>
{
public:
    small_buffer ()
    { new (&storage_.buffer_) T(); }

    explicit small_buffer (T value)
    { new (storage_.buffer_.data()) T(std::move(value)); }

    small_buffer (const small_buffer & rhs)
    { new (storage_.buffer_.data()) T(rhs.storage_.cast()); }

    small_buffer (small_buffer && rhs)
    { new (storage_.buffer_.data()) T(std::move(rhs.storage_.cast())); }

    small_buffer & operator= (const small_buffer & rhs)
    {
        storage_.cast().~T();
        new (storage_.buffer_.data()) T(rhs.storage_.cast());
        return *this;
    }

    small_buffer & operator= (small_buffer && rhs)
    {
        using std::swap;
        swap(storage_.cast(), rhs.storage_.cast());
        return *this;
    }

    ~small_buffer ()
    { storage_.cast().~T(); }

    T & value ()
    { return operator*(); }

    T & operator* ()
    { return storage_.cast(); }

    T * operator-> ()
    { return &storage_.cast(); }

private:
    detail::small_buffer_storage<T, Bytes> storage_;
};

#endif
