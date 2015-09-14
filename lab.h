struct DICT
{

    struct node
    {
        string key;
        int  value;
        node* next;
    };

    node**   table;
    u32      count;
    u32      capacity;
    void(*clrfun)(int*);
    s32(*cmpkey)(string, string);
    s32(*cmpval)(int, int);
    u32*     primes;

    DICT(int cap)
    {
        table = CALLOC(node*, cap);
        capacity = cap;
        count = 0;
    }

    DICT()
    {
        table = CALLOC(node*, 3);
        capacity = 3;
        count = 0;
        int cmp_string(string, string);
        cmpkey = cmp_string;

        primes = new u32[14 * 5]
        {
            3, 7, 11, 17, 23,
                29, 37, 47, 59, 71,
                89, 107, 131, 163, 197,
                239, 293, 353, 431, 521,
                631, 761, 919, 1103, 1327,
                1597, 1931, 2333, 2801, 3371,
                4049, 4861, 5839, 7013, 8419,
                10103, 12143, 14591, 17519, 21023,
                25229, 30293, 36353, 43627, 52361,
                62851, 75431, 90523, 108631, 130363,
                156437, 187751, 225307, 270371, 324449,
                389357, 467237, 560689, 672827, 807403,
                968897, 1162687, 1395263, 1674319, 2009191,
                2411033, 2893249, 3471899, 4166287, 4999559
        };
    }

    u32 computehash(string key)
    {
        u32 hash;
        for (hash = 0; *key != '\0'; key++)
            hash = *key + 31 * hash;
        return hash % capacity;
    }

    node* find(string key, u32& hash)
    {
        hash = computehash(key);
        for (node* e = table[hash];
                e != null;
                e = e->next)
        {
            if (cmpkey(e->key, key) == 0)
                return e;
        }
        return null;
    }

    void add(string key, int value)
    {
        // try to find the node at the specified key if it
        // doesn't exist we allocate a new node with the
        // specified key and value otherwise we clear the
        // previously existing value and assign it the new one
        u32 hash;
        node* e = find(key, hash);
        if (e)
        {
            if (clrfun) clrfun(&e->value);
        }
        else
        {
            if (count == capacity)
            {
                // keep old capacity to use in loop
                u32 oldcap = capacity;
                u32 nextcapidx = LINEAR_SEARCH(u32, primes,
                        14, capacity, cmp_u32) + 1;
                capacity = primes[nextcapidx];

                // newtable with new capacity. let's resize and
                // rehash
                node** newtable = CALLOC(node*, capacity);
                FOREACH(node* it, table, oldcap)
                {
                    while (it)
                    {
                        node* next = it->next;
                        u32 newhash = computehash(it->key);
                        if (newtable[newhash] == null)
                            newtable[newhash] = it;
                        else
                        {
                            node* tmp = newtable[newhash];
                            while (tmp->next) tmp = tmp->next;
                            tmp->next = it;
                        }
                        it->next = null;
                        it = next;
                    }
                }

                free(table);
                table = newtable;

                // recompute hash since we resized
                hash = computehash(key);
            }

            e = MALLOC(node);
            e->key = key;
            e->next = table[hash]; // point to the head
            table[hash] = e; // this node is now the new head
            count++;
        }

        e->value = value;
    }

    int operator [](string key)
    {
        u32 hash;
        node* e = find(key, hash);
        if (!e) return -1;
        return e->value;
    }
};

struct LINQ
{

    LINQ* operator -> () { return this; }

    typedef s8(*cmpfun_t)(void*, void*);

    void**     selbuf;
    void**     mainbuf;
    u32        count;
    cmpfun_t   linqcmp;

    LINQ* from(void** buf, u32 cnt, cmpfun_t cmp)
    {
        count = cnt;
        mainbuf = buf;
        linqcmp = cmp;
        return this;
    }

    void select(void* (*xform)(void*))
    {
        selbuf = RALLOC(selbuf, void*, count);
        FOR(count)
            selbuf[i] = xform(mainbuf[i]);
    }

    LINQ* where(bool(*predicate)(void*))
    {
        u32 cmpidx = count - 1;
        for (u32 i = 0;;)
        {
            void* it = mainbuf[i];
            if (predicate(it))
            {
                i++;
            }
            else
            {
                SWAP(void*, mainbuf[i], mainbuf[cmpidx]);
                cmpidx--;
                count--;
            }
            if (cmpidx < i)
                break;
        }
        return this;
    }

    LINQ* distinct()
    {
        // in:  1, 2, 1, 1, 3, 2
        // out: 1, 2, 3 (in any order)
        for (u32 i = 0, nextplace = 0, cnt = count, skip; i < cnt; i++)
        {
            skip = 0;
            void* it = mainbuf[i];
            for (s32 k = i - 1; k >= 0; k--)
            {
                if (linqcmp(mainbuf[k], it) == 0)
                {
                    skip = 1;
                    cnt--;
                    break;
                }
            }
            if (skip) continue;
            if (i == nextplace) { nextplace++; continue; }
            SWAP(void*, mainbuf[i], mainbuf[nextplace]);
            nextplace++;
        }
        return this;
    }

    LINQ* take(u32 n)
    {
        if (n > count)
            n = count;
        count = n;
        return this;
    }

    LINQ* skip(u32 n)
    {
        // 1, 2, 3, 4, 5
        // 3, 4, 5, 1, 2
        u32 swapidx = count - n;
        FOR(n)
        {
            SWAP(void*, mainbuf[i], mainbuf[swapidx]);
            swapidx++;
        }
        count -= n;
        return this;
    }

    LINQ* orderasc()
    {
        return this;
    }

    LINQ* orderdec()
    {
        return this;
    }

    LINQ* reverse()
    {
        // 1, 2, 3, 4, 5
        // 5, 4, 3, 2, 1

        // 1, 2, 3, 4
        // 4, 3, 2, 1

        for (u32 left = 0, right = count - 1;
                left < right;
                left++, right--)
            SWAP(void*, mainbuf[left], mainbuf[right]);
        return this;
    }

} linq;

#define linq_from(buf, cnt, cmp)\
    linq->from((void**)(buf), (cnt), (cmp))

struct memtable_
{
    #define TABLE_SIZE 64
    void* pointers[TABLE_SIZE];
    u32   sizes[TABLE_SIZE];
    u32   count;

    u32 getsize(void* address)
    {
        s32 index = -1;
        for (u32 i = 0; i < count; i++)
        {
            if (pointers[i] == address){
                index = i;
                break;
            }
        }
        return index == -1 ? 0 : sizes[index];
    }

    void map(void* address, u32 size)
    {
        assert(count < TABLE_SIZE);
        pointers[count] = address;
        sizes[count++] = size;
    }

} memtable;
