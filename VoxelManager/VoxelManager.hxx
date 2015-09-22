/*
Copyright (C) 2015 Vlad Mesco

This file is part of VoxelVolumizer.

VoxelVolumizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Foobar is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VoxelVolumizer.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef VOXEL_MANAGER_HXX
#define VOXEL_MANAGER_HXX

#include <cstdint>


class VoxelManager
{
#if 1
    struct alignas(32) Voxel
    {
        enum neighbours_t {
            LEFT = 0,
            FRONT = 1,
            RIGHT = 2,
            BACK = 4
        };

        Voxel(uint32_t word) : word_(word) {}
        operator uint32_t() { return word_; }

        explicit Voxel(unsigned x, unsigned y, unsigned neighbours, unsigned reserved = 0)
            : x_(x)
            , y_(y)
            , neighbours_(neighbours)
            , reserved(reserved)
        {}

        unsigned GetX() const
        {
            return x_;
        }

        void SetX(unsigned x)
        {
            x_ = x;
        }

        unsigned GetY() const
        {
            return y_;
        }

        void SetY(unsigned y)
        {
            y_ = y;
        }

        bool IsNeighbourSet(neighbours_t neighbour) const
        {
            return neighbours_ & neighbour;
        }

        void SetNeighbour(neighbours_t neighbour, bool set)
        {
            
            neighbours_ = (neighbours_ & (~neighbour)) | (set * neighbour);
        }

    private:
        union {
            struct {
                int x_ : 13;
                int y_ : 13;
                int reserved_ : 2;
                int neighbours_ : 4;
            };
            uint32_t word_;
        }
    };
#else
    struct alignas(32) Voxel
    {
        constexpr uint32_t X_MASK = 0xFFF80000u;
        constexpr uint32_t Y_MASK = 0x0007FFE0u;
        constexpr uint32_t X_OFF = 19u;
        constexpr uint32_t Y_OFF = 6u;
        constexpr uint32_t XY_SIZE_MASK = 0x1FFFu;

        Voxel(uint32_t word) : word_(word) {}
        operator uint32_t() { return word_; }

        unsigned GetX() const
        {
            unsigned ret =
                (word_ & X_MASK) >> X_OFF;
            return ret;
        }

        void SetX(unsigned x)
        {
            word_ &= ~X_MASK;
            word_ |= (x & XY_SIZE_MASK) << X_OFF;
        }

        unsigned GetY() const
        {
            unsigned ret =
                (word_ & Y_MASK) >> Y_OFF;
            return ret;
        }

        void SetY(unsigned)
        {
            word_ &= ~Y_MASK
            word_ |= (y & XY_SIZE_MASK) << Y_OFF;
        }

        bool IsNeighbourSet(neighbours_t neighbour) const
        {
            return word_ & neighbour;
        }

        void SetNeighbour(neighbours_t neighbour, bool set)
        {
            word_ = (word_ & (~neighbour)) | (set * neighbour);
        }

    private:
        /*
        int x : 13;
        int y : 13;
        int reserved : 2;
        int neighbours : 4;
        */
        uint32_t word_;
    };
#endif
    
    struct VoxelFileBlockCacheManager
    {
        // first is last recently used; least recent at the end
        // when memory usage goes above MB, the least recently used get wiped
        std::list<VoxelFileBlock*> m_cachedAndMRU;
        void SetMemory(uint32_t MB);
        void Accessed(VoxelFileBlock*);
    };

    struct VoxelFileBlock
    {
        VoxelFileBlock(FILE*, int N, size_t offset); // TODO offset should be offset_t?
        VoxelFileBlock(FILE*, size_t offset); // TODO offset should be offset_t?
        ~VoxelFileBlock()
        {
            if(GetLinked()) delete GetLinked();
        }
        VoxelFileBlock* GetLinked();
        VoxelFileBlock* CreateLinked(int N);
        bool HasPlane(int pId);
        Voxel GetVoxel(int pId, int idx);
        void SetVoxel(int pId, int idx, Voxel);
        uint32_t GetN() const;
        void WipeCache();
    private:
        uint32_t* page_;
        std::vector<std::pair<uint32_t, uint32_t>> index_;
        VoxelFileBlock* linked_;
    };

private:
    VoxelFileBlock* fileHead_;
    VoxelFileBlockCacheManager cache_;

public:
    VoxelManager(FILE*, int preferrend_N = 1000, bool forceWipe = false);
    ~VoxelManager();
    void SetVoxel(int pId, int i, int j, bool filled);
    bool GetVoxelFill(int pId, int i, int j);
};

#endif
