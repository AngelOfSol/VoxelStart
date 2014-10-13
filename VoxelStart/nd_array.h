#pragma once

#include <vector>
#include <array>

template <class Parent, int D>
class nd_view
{
public:
	using sub_view = nd_view<Parent, D - 1>;
	nd_view(Parent& parent, int offset)
		: m_parent(parent)
		, m_offset(offset)
	{

	}
	sub_view operator [](int index)
	{
		return sub_view(this->m_parent, this->m_offset + index * this->m_parent.chunk_size<D>());
	}
	sub_view operator [](int index) const
	{
		return sub_view(this->m_parent, this->m_offset + index * this->m_parent.chunk_size<D>());
	}
private:
	int const m_offset;
	Parent& m_parent;
};

template <class Parent>
class nd_view<Parent, 1>
{
public:
	nd_view(Parent& parent, int offset)
		: m_parent(parent)
		, m_offset(offset)
	{

	}
	auto operator [](int index) -> typename Parent::reference_type
	{
		return this->m_parent.get_absolute(index + this->m_offset);
	}
	auto operator [](int index) const -> typename Parent::const_reference_type
	{
		return this->m_parent.get_absolute(index + this->m_offset);
	}
private:
	int const m_offset;
	Parent& m_parent;
};

template <class Parent, int D>
class const_nd_view
{
public:
	using sub_view = const_nd_view<Parent, D - 1>;
	const_nd_view(const Parent& parent, int offset)
		: m_parent(parent)
		, m_offset(offset)
	{

	}
	sub_view operator [](int index)
	{
		return sub_view(this->m_parent, this->m_offset + index * this->m_parent.chunk_size<D>());
	}
	sub_view operator [](int index) const
	{
		return sub_view(this->m_parent, this->m_offset + index * this->m_parent.chunk_size<D>());
	}
private:
	int const m_offset;
	const Parent& m_parent;
};
template <class Parent>
class const_nd_view<Parent, 1>
{
public:
	const_nd_view(const Parent& parent, int offset)
		: m_parent(parent)
		, m_offset(offset)
	{

	}
	auto operator [](int index) const -> typename Parent::const_reference_type
	{
		return this->m_parent.get_absolute(index + this->m_offset);
	}
private:
	int const m_offset;
	const Parent& m_parent;
};

template <typename T, int D>
class nd_array
{
public:
	using dim_array = std::array<int, D>;
	using sub_view = nd_view<nd_array, D - 1>;
	using const_sub_view = const_nd_view<nd_array, D - 1>;
	using reference_type = T&;
	using const_reference_type = T const&;

	template <typename... Sizes>
	nd_array(Sizes... dimensions)
	{
		int dims[] = { dimensions... };

		this->m_size = 1;

		for (int i = D - 1; i >= 0; i--)
		{
			this->m_sizes[i] = dims[i];
			this->m_size *= dims[i];
			if (i == D - 1)
			{
				this->m_chunk_sizes[i] = 1;
			}
			else
			{
				this->m_chunk_sizes[i] = this->m_chunk_sizes[i + 1] * dims[i + 1];
			}
		}
		this->m_data.resize(this->chunk_size<D>() * dims[D - 1]);
	}

	sub_view operator [](int index)
	{
		return sub_view(*this, index * this->chunk_size<D>());
	}
	const_sub_view operator [](int index) const
	{
		return const_sub_view(*this, index * this->chunk_size<D>());
	}

	T& get_absolute(int index)
	{
		return this->m_data[index];
	}
	T const& get_absolute(int index) const
	{
		return this->m_data[index];
	}

	template <int TD>
	int chunk_size() const
	{
		return this->m_chunk_sizes[D - TD];
	}

	int size() const
	{
		return this->m_size;
	}

	template <int TD>
	int dim_size() const
	{
		return this->m_sizes[TD];
	}


private:
	std::vector<T> m_data;
	dim_array m_sizes;
	dim_array m_chunk_sizes;
	int m_size;
};
