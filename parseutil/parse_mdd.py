# Portions of this software derived from code in the prjxray-bram-patch open source repository (https://github.com/symbiflow/prjxray-bram-patch).  
# Use of that source code is governed by a ISC-style
# license that can be found in the COPYING file in that repository or at
# https://opensource.org/licenses/ISC
#
import sys
from pathlib import Path
import pathlib

WIDTH_MISMATCH_FLAG = False


class Cell:
    def __init__(self, cell):
        self.cell_name = cell['CELL']
        self.type = cell['CELLTYPE']
        self.tile = cell['TILE']
        self.placement = cell['LOC'].split('_')[1]
        self.write_style = cell['MEM.PORTA.DATA_BIT_LAYOUT']
        self.pbits = int(cell['MEM.PORTA.DATA_BIT_LAYOUT'].split('_')[0][1:])
        self.dbits = int(cell['MEM.PORTA.DATA_BIT_LAYOUT'].split('_')[1][1:])
        self.ram_name = cell['RTL_RAM_NAME']
        # self. = cell['RAM_EXTENSION_A']
        self.mode = cell['RAM_MODE']
        self.width = int(cell['READ_WIDTH_A'])
        self._rwa = int(cell['READ_WIDTH_A'])
        self._wwa = int(cell['READ_WIDTH_B'])
        self._rwb = int(cell['WRITE_WIDTH_A'])
        self._wwb = int(cell['WRITE_WIDTH_B'])
        # self._offset = int(cell['RAM_OFFSET'])
        self.addr_beg = int(cell['BRAM_ADDR_BEGIN'] if cell['BRAM_ADDR_BEGIN'] != 'NONE' else cell['RAM_ADDR_BEGIN'])
        self.addr_end = int(cell['BRAM_ADDR_END'] if cell['BRAM_ADDR_END'] != 'NONE' else cell['RAM_ADDR_END'])
        self.slice_beg = int(cell['BRAM_SLICE_BEGIN'] if cell['BRAM_SLICE_BEGIN'] != 'NONE' else cell['RAM_SLICE_BEGIN'])
        self.slice_end = int(cell['BRAM_SLICE_END'] if cell['BRAM_SLICE_END'] != 'NONE' else cell['RAM_SLICE_END'])
        self.offset = cell["STARTINGOFFSET"]
        # self. = cell['RAM_ADDR_BEGIN']
        # self. = cell['RAM_ADDR_END']
        # self. = cell['RAM_SLICE_BEGIN']
        # self. = cell['RAM_SLICE_END']
        self.INIT_LIST = []
        self.INITP_LIST = []
        self.INIT = []
        self.INITP = []
        # print(f'{self.type} p{self.pbits}_d{self.dbits}')

    def toString(self):
        s = "{} {} {} {} {} {} {}".format(
            self.cell_name, self.type, self.tile, self.placement,
            self.ram_name, self.addr_beg, self.slice_beg
        )
        return s


def insolateUniqueLogical(mdd, verbose=False):
    if isinstance(mdd, pathlib.Path):
        mdd = str(mdd)

    temp_mdd_data, part = read_mdd(mdd)

    mdd_data = [
        '/'.join(m.cell_name.split('/')[:-1]) + '/' +
        m.ram_name for m in temp_mdd_data
    ]

    return set(mdd_data), temp_mdd_data, part


def printRelatedBRAM(dir, logical, i, fullData):

    mdd_data = [
        m for m in fullData
        # Reassemble RAM name by removing the ram_reg_*_* part from it.
        # The user wants to specify 'mem/ram' instead of mem/ram_reg_0_0/ram
        if '/'.join(m.cell_name.split('/')[:-1]) + '/' +
           m.ram_name == logical
    ]

    with open(dir + '/' + 'mem_' + str(i) + ".bram", 'w') as f:
        for m in mdd_data:
            f.write(m.type + '_' + m.placement + '\n')


def readAndFilterMDDData(selectedMemToPatch, fullData, verbose=False):
    mdd_data = [
        m for m in fullData
        # Reassemble RAM name by removing the ram_reg_*_* part from it.
        # The user wants to specify 'mem/ram' instead of mem/ram_reg_0_0/ram
        if '/'.join(m.cell_name.split('/')[:-1]) + '/' +
           m.ram_name == selectedMemToPatch
    ]
    if len(mdd_data) == 0:
        print(
            "No memories found in MDD file corresponding to {}, aborting.".
                format(selectedMemToPatch)
        )
        exit(1)

    if verbose:
        print("Memories to be patched ({}):".format(len(mdd_data)))
        for l in mdd_data:
            print("  " + l.toString())
        print("")

    return mdd_data


def main():
    mdd_name = sys.argv[1]
    # print(f'Reading {mdd_name}')
    read_mdd(mddfile=mdd_name)


def read_mdd(mddfile):
    cells = {}
    part = ''
    with open(mddfile, 'r') as f:
        addr = ''
        for ln in f:
            ln = ln.strip()
            ln = ln.split(' ')
            if ln[0] == 'CELL':
                addr = ln[1]
                cells[addr] = {}
                cells[addr]['CELL'] = addr
            elif ln[0] == 'ENDCELL':
                addr = ''
                continue
            elif addr != '':
                cells[addr][ln[0]] = ln[1]
            elif ln[0] == 'PART':
                part = ln[1]

                # print(f'Assigned {ln[1]} to parameter {ln[0]} for {addr}')
    # for key, cell in cells.items():
    #     print(f'CELL {key}')
    #     for param, val in cell.items():
    #         print(f'  {param:<27} {val}')
    cell_list = []
    for cell in cells.values():
        newcell = Cell(cell)
        cell_list.append(newcell)
    # print(type(mddfile))
    get_width(cell_list)
    if WIDTH_MISMATCH_FLAG and type(mddfile) == pathlib.PosixPath:
        (mddfile.parent / 'WIDTH_MISMATCH').touch()

    return cell_list, part


def get_width(mdd):
    last_pos = 0
    for cell in mdd:
        end = cell.slice_end
        if end > last_pos:
            last_pos = end
    slicewid = last_pos + 1
    readwid = mdd[0].width
    if readwid > slicewid:
        return readwid
    else:
        return slicewid
    # wid = 0
    # oldwid = mdd[0].width
    # for cell in mdd:
    #     wid = cell.width
    #     if oldwid != wid:
    #         print('WIDTH MISMATCH')
    #         WIDTH_MISMATCH_FLAG = True
    #     oldwid = wid
    # return wid


if __name__ == "__main__":
    main()
