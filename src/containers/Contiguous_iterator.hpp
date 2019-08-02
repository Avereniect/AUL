//
// Created by avereniect on 8/1/19.
//

#ifndef AUL_CONTIGUOUS_ITERATOR_HPP
#define AUL_CONTIGUOUS_ITERATOR_HPP

template<typename T, typename Alloc_types, bool is_const>
class Contiguous_iterator {
public:

    //-----------------------------------------------------
    // Type aliases
    //-----------------------------------------------------

    using value_type = typename Allocator_types::value_type;
    using difference_type = typename Allocator_types::difference_type;
    using pointer = typename std::conditional<is_const,
        typename Allocator_types::const_pointer,
        typename Allocator_types::pointer
    >::type;
    using reference = typename std::conditional<is_const, const value_type&, value_type&>::type;
    using iterator_category = std::random_access_iterator_tag;

    //-----------------------------------------------------
    // -ctors
    //-----------------------------------------------------



private:

};


#endif //AUL_CONTIGUOUS_ITERATOR_HPP
