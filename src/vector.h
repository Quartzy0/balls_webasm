#ifndef BALLS_WEBASM_VECTOR_H
#define BALLS_WEBASM_VECTOR_H

#define VECTOR_NEW(type, name, size) type name [size];	\
	uint16_t vector_count_ ## name = 0;										\
	uint16_t vector_free_indices_ ## name [size];						\
	uint16_t vector_last_index_ ## name = -1;							\
	const uint16_t vector_capacity_ ## name = size

#define VECTOR_COUNT(name) vector_count_ ## name

#define VECTOR_REMOVE(name, index) do{			\
		vector_last_index_ ## name ++;			\
		vector_free_indices_ ## name [vector_last_index_ ## name] = index; \
		vector_count_ ## name --;										\
		memset(&name [index], 0, sizeof(*name));																\
	}while(0)

#define VECTOR_ADD(name, out, indexOut) do{				\
		if(vector_last_index_ ## name == ((uint16_t) -1)){ \
			out = &name [vector_count_ ## name];	\
			indexOut = vector_count_ ## name;					\
		}else {													\
			out = &name [vector_free_indices_ ## name [vector_last_index_ ## name]]; \
			indexOut = vector_free_indices_ ## name [vector_last_index_ ## name];\
			vector_last_index_ ## name --;												\
		}																	\
		vector_count_ ## name ++;																	\
	}while(0)

#define VECTOR_ADD_A(name, out, indexOut) do{				\
		if(vector_last_index_ ## name == ((uint16_t) -1)){ \
			name [vector_count_ ## name] = out;	\
			indexOut = vector_count_ ## name;					\
		}else {													\
			name [vector_free_indices_ ## name [vector_last_index_ ## name]] = out; \
			indexOut = vector_free_indices_ ## name [vector_last_index_ ## name];\
			vector_last_index_ ## name --;												\
		}																	\
		vector_count_ ## name ++;																	\
	}while(0)

#define VECTOR_CLEAR(name) do{					\
		memset(name, 0, vector_capacity_ ## name * sizeof(*name)); \
		vector_count_ ## name; \
		memset(vector_free_indices_ ## name, 0, vector_capacity_ ## name * sizeof(*vector_free_indices_ ## name)); \
		vector_last_index_ ## name = -1;																\
	}while(0)

#endif
