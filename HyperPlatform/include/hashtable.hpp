#pragma once
#include "global.hpp"
#include "vector.hpp"

namespace std
{
	inline int prime_list_size = 20;
	inline unsigned long prime_list[]
	{
		53,97,193,389,769,
		1543,3079,6151,12289,24593,
		49157,98317,196613,393241,786433,
		1572869,3145739,6291469,12582917,25165843
	};

	template <typename key, typename value>
	struct _hashtable_node
	{
		// ��Ϊֻ����һ���ڵ��ָ��
		// �Դ˷�װ�ĵ�����ֻ֧��ǰ����������֧�ֺ��ˣ�Ҳû�����������
		_hashtable_node* next;
		key k;
		value val;

	};


	template<typename key, typename value, typename HashFcn>
	struct __hashtable_iterator
	{
		using reference = value&;
		using pointer = value*;
		using node = _hashtable_node<key, value>;

		node* cur;

		__hashtable_iterator(node* n) { cur = n; }

		reference operator*()const { return cur->val; }


	};

	template<class key,
		class value,
		// type_traits.hpp�ṩһ����Ĭ�ϵĹ�ϣ����
		class HashFcn = std::hash<key>>
		class hashtable
	{
		using node = _hashtable_node<key, value>;
		using size_type = size_t;
		using iterator = __hashtable_iterator<key, value, HashFcn>;

	private:
	public:
		std::vector<node*> buckets;

		// ѡ��buckets�Ĵ�С
		void initialize_buckets(size_type n)
		{
			auto next_size = [n]() {
				//���ش���n���������
				for (auto prime : prime_list)
				{
					if (prime > n)
						return prime;
				}
				};
			buckets.resize(next_size());
		}

		// Ĭ�Ϲ��캯��
		hashtable()
		{
			initialize_buckets(20000);
		}

		// ��ʾ���֧�ֵ�buckets�Ƕ���
		unsigned long max_buckets()
		{
			return prime_list[prime_list_size - 1];
		}

		// ȷ��ĳ��obj�÷��ĸ�λ��,�����±�(����)
		size_t bkt_num(key obj)
		{
			return HashFcn()(obj) % buckets.capacity();
		}

		// ��hashtable�в�������
		iterator insert(key k, value val)
		{
			//
			//������������ص�
			//
			const size_type n = bkt_num(k);
			node* first = buckets[n];
			if (!first)
			{
				//
				//��ص�Ϊ�գ�ֱ��дvalue
				//
				first = new node;
				first->val = val;
				first->k = k;
			}
			else
			{
				//��ϣ��ͻ��
				
			}

			return first;
		}

	};










