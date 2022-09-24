#pragma once

#ifndef BIT_HPP_
#define BIT_HPP_

namespace sen {

template<int index, typename T>
static inline auto bit(T data) -> bool { return data & (1 << index); }

}

#endif //BIT_HPP_
