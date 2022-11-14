from enum import IntEnum, Flag


class TypeLocation:
    def __init__(self, begin: int, end: int, alignment: int):
        self.begin = begin
        self.end = end
        self.alignment = alignment


def align(value, alignment):
    align = value % alignment

    if align:
        value += alignment - align
    return value


class SwapType(IntEnum):
    DontSwap = 0
    S16 = 1
    S32 = 2
    S64 = 3


class NamedType:
    def __init__(self, name: str, size=0, alignment=0, swap_type=SwapType.DontSwap):
        self.name = name
        self.size = size
        self.alignment = alignment
        self.swap_type = swap_type

    def get_location(self, cur_offset, settings):
        cur_offset = align(cur_offset, self.alignment)

        return TypeLocation(cur_offset, cur_offset + self.size, self.alignment)

    def get_swap_type(self, settings):
        return self.swap_type

    def str_decl(self):
        return self.name


class Template():
    def __init__(self, name: str):
        self.name = name
        self.id = 'class'
        self.template: Template = None

    def str_decl(self):
        return self.name

    def __str__(self):
        if self.template:
            return 'template<%s> %s %s' % (self.template, self.id, self.name)
        else:
            return '%s %s' % (self.id, self.name)


class TYPES:
    bool = NamedType('bool', 1, 1)
    uint8 = NamedType('uint8', 1, 1)
    int8 = NamedType('int8', 1, 1)
    char = NamedType('char', 1, 1)
    uint16 = NamedType('uint16', 2, 2, SwapType.S16)
    int16 = NamedType('int16', 2, 2, SwapType.S16)
    uint32 = NamedType('uint32', 4, 4, SwapType.S32)
    int32 = NamedType('int32', 4, 4, SwapType.S32)
    uint64 = NamedType('uint64', 8, 8, SwapType.S64)
    int64 = NamedType('int64', 8, 8, SwapType.S64)
    float = NamedType('float', 4, 4, SwapType.S32)
    double = NamedType('double', 8, 8, SwapType.S64)
    Vector4A16 = NamedType('Vector4A16', 16, 16)
    Vector = NamedType('Vector', 12, 4)
    Vector4 = NamedType('Vector4', 16, 4)
    Vector2 = NamedType('Vector2', 8, 4)


class PermSettings:
    def __init__(self, permutators):
        self.permutators = permutators
        self.current_perm = None
        self.pointer_x64 = False
        self.gnu_layout = False
        self.swap_pointers = False


class LayoutFlags(Flag):
    Null = 0
    Ptr = 1
    Padding = 2


class MainSettings:
    def __init__(self):
        self.permutators = []
        self.pointer_x64 = False
        self.pointer_x86 = False
        self.ignore_pointer_endianness = False
        self.class_layout_gnu = False
        self.class_layout_msvc = True
        self.last_layout_flags = LayoutFlags.Null


class TemplateLink:
    def __init__(self, template, *args):
        self.template = template
        self.args = args

    def get_location(self, cur_offset, settings: PermSettings):
        loc: TypeLocation = self.template.get_location(0, settings)

        align = cur_offset % loc.alignment

        if align:
            cur_offset += loc.alignment - align

        loc.begin = cur_offset
        loc.end += cur_offset

        return loc

    def str_decl(self):
        return '%s<%s>' % (self.template.name, ', '.join(item.str_decl() for item in self.args))


class Pointer(TemplateLink):
    def __init__(self, type):
        super(Pointer, self).__init__(NamedType('Pointer'), type)

    def get_location(self, cur_offset, settings: PermSettings):
        repr = TYPES.uint64 if settings.pointer_x64 else TYPES.uint32
        return repr.get_location(cur_offset, settings)

    def get_swap_type(self, settings: PermSettings):
        if settings.swap_pointers:
            return SwapType.S64 if settings.pointer_x64 else SwapType.S32
        return SwapType.DontSwap

    def cpp_submethod(self, context, parent):
        ptr_type = self.args[0]
        stuff = {
            'name': parent.name,
            'type': ptr_type.str_decl(),
            'const': ''
        }

        template = '''Pointer<{type}> %s() {{
    int16 off = m({name}); if (off == -1) return {{nullptr, lookup}};
    return {{data + off, lookup}};
  }}
'''
        context[0].add_unique_line(template.format_map(stuff),
                                    None, parent.func + 'Ptr', parent.perm)

        if type(ptr_type) == NamedType:
            template = '''{const}{type} *%s() {const}{{
    int16 off = m({name}); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<{type}**>(data + off);
    return *reinterpret_cast<es::PointerX86<{type}>*>(data + off);
  }}
'''
        else:
            template = '''Iterator<{type}> %s() {const}{{
    int16 off = m({name}); if (off == -1) return {{nullptr, lookup}};
    if (layout->ptrSize == 8) return {{*reinterpret_cast<char**>(data + off), lookup}};
    return {{*reinterpret_cast<es::PointerX86<char>*>(data + off), lookup}};
  }}
'''
        non_const_line = template.format_map(stuff)
        stuff['const'] = 'const '
        context[0].add_unique_line(non_const_line,
                                    template.format_map(stuff), parent.func, parent.perm)


class InlineArray(TemplateLink):
    def __init__(self, type_, count):
        class TemplateType:
            def __init__(self, type_, count):
                self.type = type_
                self.count = count
            def get_location(self, cur_offset, settings):
                sub_loc: TypeLocation = self.type.get_location(cur_offset, settings)
                sub_loc.end = (sub_loc.end - sub_loc.begin) * self.count

                return sub_loc
        class Counter:
            def __init__(self, count):
                self.count = count
            def str_decl(self):
                return str(self.count)

        super(InlineArray, self).__init__(TemplateType(type_, count), type_, Counter(count))
        self.type = type_
        self.count = count

    def get_swap_type(self, settings):
        return SwapType.DontSwap

    def cpp_submethod(self, context, parent):
        stuff = {
            'name': parent.name,
            'type': 'std::span<%s>' % (self.type.str_decl()),
            'subtype': self.type.str_decl(),
            'count': str(self.count),
        }
        template = None
        if type(self.type) == NamedType:
            template = '{type} %s() const {{ return m({name}) == -1 ? {type}{{}} : {type}{{reinterpret_cast<{subtype}*>(data + m({name})), {count}}}; }}\n'
        else:
            template = 'LayoutedSpan<{subtype}> %s() const {{ return {{{{m({name}) == -1 ? nullptr : data + m({name}), lookup}}, {count} }}; }}\n'

        context[0].add_unique_line(
            template.format_map(stuff), None, parent.func, parent.perm)


class ClassPatchType(IntEnum):
    insert_after = 0
    replace = 1
    delete = 3
    append = 4
    prepend = 5


class ClassPatch:
    def __init__(self, ref: str, *args):
        self.ref = ref
        self.args = args

    def collect_member_names(self, parent: list):
        for a in self.args:
            if type(a) == str or type(a) == ClassPatchType:
                continue
            if type(a) == Inheritable:
                parent.append('base' + a.data.name)
                continue
            parent = a.append_names(parent)
        return parent

    def patch(self, members: list):
        current_operator = None
        after_iter = None
        args_iter = iter(self.args)

        while True:
            cur_iter = next(args_iter, None)

            if type(cur_iter) == ClassPatchType:
                current_operator = cur_iter
                if cur_iter == ClassPatchType.insert_after:
                    ref_name = next(args_iter, None)
                    for m in range(0, len(members)):
                        if members[m].name == ref_name:
                            after_iter = m
                            break
            elif type(cur_iter) == Inheritable:
                continue
            elif not cur_iter:
                break
            else:
                if current_operator == ClassPatchType.replace:
                    for m in range(0, len(members)):
                        if members[m].name == cur_iter.name:
                            members[m] = cur_iter
                            break
                elif current_operator == ClassPatchType.append:
                    members.append(cur_iter)
                elif current_operator == ClassPatchType.delete:
                    for m in members:
                        if m.name == cur_iter:
                            members.remove(m)
                            break
                elif current_operator == ClassPatchType.insert_after:
                    after_iter = after_iter + 1
                    members.insert(after_iter, cur_iter)
                elif current_operator == ClassPatchType.prepend:
                    members.insert(0, cur_iter)

    def patch_bases(self, members: list):
        current_operator = None
        after_iter = None
        args_iter = iter(self.args)

        while True:
            cur_iter = next(args_iter, None)

            if type(cur_iter) == ClassPatchType:
                current_operator = cur_iter
                if cur_iter == ClassPatchType.insert_after:
                    ref_name = next(args_iter, None)
                    for m in range(0, len(members)):
                        if members[m].name == ref_name:
                            after_iter = m
                            break
            elif not cur_iter:
                break
            elif type(cur_iter) != Inheritable:
                continue
            else:
                if current_operator == ClassPatchType.replace:
                    for m in range(0, len(members)):
                        if members[m].name == cur_iter.name:
                            members[m] = cur_iter.data
                            break
                elif current_operator == ClassPatchType.append:
                    members.append(cur_iter.data)
                elif current_operator == ClassPatchType.delete:
                    for m in members:
                        if m.name == cur_iter.data:
                            members.remove(m)
                            break
                elif current_operator == ClassPatchType.insert_after:
                    after_iter = after_iter + 1
                    members.insert(after_iter, cur_iter.data)
                elif current_operator == ClassPatchType.prepend:
                    members.insert(0, cur_iter)


class LayoutItem:
    def __init__(self, *args):
        self.version_begin = args[0]
        self.version_end = args[1]
        self.ptr_size = args[2]
        self.gnu_layout = args[3]
        self.total_size = args[4]
        self.offsets = args[5]
        self.swaps = args[6]

    def __eq__(self, other):
        same_size = self.total_size == other.total_size
        same_offsets = self.offsets == other.offsets
        same_swaps = self.swaps == other.swaps
        return same_size and same_offsets and same_swaps

    def str_cpp(self):
        return '{{{{%s, %s, %s, %s}}, %s}, {%s}, {%s}}' % \
            (self.version_begin, self.version_end, self.ptr_size, self.gnu_layout, self.total_size, str(self.offsets)[1:-1],  str(self.swaps)[1:-1].replace('\'', ''))


class ClassData:
    def __init__(self, name: str):
        self.name = name
        self.templates: 'list[Template]' = []
        self.inherits: 'list[ClassData]' = []
        self.members: 'list[ClassMember]' = []
        self.patches: 'list[ClassPatch]' = []

    def str_members(self, patch):
        members = self.members
        if patch:
            members = self.members.copy()
            for p in self.patches:
                p.patch(members)
                if p == patch:
                    break

        return '\n'.join('  ' + item.str_decl() for item in members)

    def str_def(self, settings: MainSettings, perm: Template):
        def signature(filter_perms=False):
            rval = ''
            if len(self.templates):
                rval = rval + \
                    'template<%s>\n' % ', '.join(str(item)
                                                 for item in self.templates if not item.id in settings.permutatorss or not filter_perms)
            return '%sstruct %s' % (rval, self.name)

        perm_data: list = settings.permutatorss[perm.id]

        def members(patch=None):
            rval = ''
            inherits = self.inherits
            if patch:
                inherits = self.inherits.copy()
                for p in self.patches:
                    p.patch_bases(inherits)
                    if p == patch:
                        break

            for i in inherits:
                def get_local_patch():
                    patch_index = None
                    for pd in range(len(perm_data)):
                        if perm_data[pd] == patch.ref:
                            patch_index = pd
                            break

                    if not patch_index:
                        return None

                    for pi in range(patch_index, 0, -1):
                        for ip in i.patches:
                            if (ip.ref == perm_data[pi]):
                                return ip
                rval = rval + i.str_members(get_local_patch())
            if len(inherits):
                rval = rval + '\n'
            return rval + self.str_members(patch)

        if not len(self.patches):
            return '%s {\n%s\n};\n' % (signature(), members())

        retval = signature(True) + ';\n' + signature(True) + \
            '<%s>' % perm_data[0] + ' {\n' + members() + '\n};\n'

        for p in self.patches:
            retval = retval + \
                signature(True) + '<%s>' % p.ref + \
                ' {\n' + members(p) + '\n};\n'

        return retval

    def collect_member_names(self, parent):
        for i in self.inherits:
            parent.append('base' + i.name)

        for m in self.members:
            parent = m.append_names(parent)

        for p in self.patches:
            parent = p.collect_member_names(parent)

        return parent

    def gen_enum_cpp(self):
        members = self.collect_member_names(list())
        members.sort()
        return 'enum Members {\n  %s};' % '  '.join(m + ',\n' for m in members + ['_count_'])

    def collect_members(self, perm_data, p):
        patch_index = perm_data.index(p)
        inherits = self.inherits.copy()
        for ip in self.patches:
            if not ip.ref in perm_data or perm_data.index(ip.ref) > patch_index:
                break
            ip.patch_bases(inherits)

        members = self.members.copy()
        for mp in self.patches:
            if not mp.ref in perm_data or perm_data.index(mp.ref) > patch_index:
                break
            mp.patch(members)

        return inherits, members

    def collect_unique_members(self, settings: MainSettings):
        all_members = {}
        for p in settings.permutators:
            inherits, members = self.collect_members(settings.permutators, p)
            for i in inherits:
                cname = 'base' + i.name
                if not cname in all_members:
                    all_members[cname] = []
                dit_item = all_members[cname]
                if not i in dit_item:
                    i.perm = p
                    dit_item.append(i)

            for m in members:
                if not m.name in all_members:
                    all_members[m.name] = []
                dit_item = all_members[m.name]
                if not m in dit_item and type(m) != ClassData:
                    m.perm = p
                    dit_item.append(m)
        return all_members

    def gen_interface_cpp(self, settings: MainSettings):
        members = self.collect_unique_members(settings)
        template = \
            '''struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {%s}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  %s

  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};'''

        class Ctx:
            def __init__(self):
                self.methods = {}
                self.lines = []
                self.settings = settings

            def add_unique_line(self, line, const_line, method_name, perm):
                if not method_name in self.methods:
                    fline = line % method_name
                    self.methods[method_name] = [fline]
                    self.lines.append(fline)
                    if const_line:
                        fline = const_line % method_name
                        self.methods[method_name].append([fline])
                        self.lines.append(fline)
                else:
                    lines = self.methods[method_name]
                    fline = line % method_name
                    if not fline in lines:
                        fline = line % (method_name + perm)
                        lines.append(fline)
                        self.lines.append(fline)
                        if const_line:
                            fline = const_line % (method_name + perm)
                            lines.append(fline)
                            self.lines.append(fline)

        context = (Ctx(), Ctx())
        [[t.cpp_method(context) for t in types] for types in members.values()]

        retval = '  '.join('  '.join(c.lines) for c in context)
        flags = []

        if settings.last_layout_flags:
            flags = [s[s.find('.') + 1:]
                     for s in str(settings.last_layout_flags).split('|')]

        return template % (' ,'.join('LookupFlag::' + f for f in flags), retval)

    def gen_offset_table(self, settings: MainSettings):
        member_names = self.collect_member_names(list())
        member_names.sort()
        perm_sets = PermSettings(settings.permutators)
        perm_sets.swap_pointers = not settings.ignore_pointer_endianness
        layout_flags = LayoutFlags.Null

        def get_offsets():
            lretval = []
            for p in settings.permutators:
                perm_sets.current_perm = p
                inherits, members = self.collect_members(
                    settings.permutators, p)
                cur_offset = 0
                offsets = [None] * len(member_names)
                swaps = [SwapType.DontSwap] * len(member_names)

                for i in inherits:
                    loc = i.get_location(cur_offset, perm_sets)
                    cur_offset = loc.end - \
                        (loc.padding if perm_sets.gnu_layout else 0)
                    idx = member_names.index('base' + i.name)
                    offsets[idx] = loc.begin

                for m in members:
                    offsets, cur_offset = m.append_offsets(
                        offsets, cur_offset, member_names, perm_sets)
                    swaps = m.append_swaps(swaps, member_names, perm_sets)
                location = self.get_location(0, perm_sets)
                ptr_size = 8 if perm_sets.pointer_x64 else 4
                lretval.append(LayoutItem(p, p, ptr_size, 1 if perm_sets.gnu_layout else 0, location.end,
                                          [o if o != None else -1 for o in offsets], [s if s else SwapType.DontSwap for s in swaps]))

            return lretval

        def perm_offsets_pointer():
            x64_offsets = []
            x86_offsets = []
            if settings.pointer_x64:
                perm_sets.pointer_x64 = True
                x64_offsets = get_offsets()
            if settings.pointer_x86:
                perm_sets.pointer_x64 = False
                x86_offsets = get_offsets()
            if x86_offsets == x64_offsets:
                return x64_offsets
            if settings.pointer_x64 and settings.pointer_x86:
                nonlocal layout_flags
                layout_flags = layout_flags | LayoutFlags.Ptr
            return x64_offsets + x86_offsets

        msvc_offsets = []
        gnu_offsets = []
        if settings.class_layout_msvc:
            perm_sets.gnu_layout = False
            msvc_offsets = perm_offsets_pointer()
        if settings.class_layout_gnu:
            perm_sets.gnu_layout = True
            gnu_offsets = perm_offsets_pointer()

        if settings.class_layout_msvc and settings.class_layout_gnu:
            if msvc_offsets != gnu_offsets:
                layout_flags = layout_flags | LayoutFlags.Padding
            else:
                gnu_offsets = []
        settings.last_layout_flags = layout_flags
        total = msvc_offsets + gnu_offsets
        total_optimized = []

        # Apply RLE sort by version
        for t in total:
            if len(total_optimized):
                last_t = total_optimized[-1]
                idx1 = settings.permutators.index(t.version_begin)
                idx2 = settings.permutators.index(last_t.version_end)

                if idx1 >= idx2 and last_t == t:
                    last_t.version_end = t.version_begin
                    continue

            total_optimized.append(t)

        return total_optimized

    def gen_table_cpp(self, settings):
        tbl = self.gen_offset_table(settings)
        for t in tbl:
            swaps = t.swaps
            nswaps = []
            cswap = 0
            for s in range(len(swaps)):
                md = s % 8
                if s and not md:
                    nswaps.append(hex(cswap))
                    cswap = 0
                cswap = cswap | swaps[s] << (md * 2)
            if len(swaps) % 8 or cswap:
                nswaps.append(hex(cswap))
            t.swaps = nswaps
        hdr = 'static const std::set<ClassData<_count_>> LAYOUTS {\n  '
        return hdr + ',\n  '.join(s.str_cpp() for s in tbl) + '\n};'

    def get_location(self, cur_offset, settings: PermSettings):
        inherits, members = self.collect_members(
            settings.permutators, settings.current_perm)
        cur_loc = TypeLocation(cur_offset, cur_offset, 0)
        max_align = 0
        min_begin = cur_offset

        for i in inherits:
            cur_loc = i.get_location(cur_loc.end, settings)
            cur_loc.end = cur_loc.end - \
                (cur_loc.padding if settings.gnu_layout else 0)
            max_align = max(max_align, cur_loc.alignment)
            min_begin = min(min_begin, cur_loc.begin)

        for m in members:
            cur_loc = m.get_location(cur_loc.end, settings)
            max_align = max(max_align, cur_loc.alignment)
            min_begin = min(min_begin, cur_loc.begin)

        cur_loc.alignment = max_align
        cur_loc.begin = min_begin
        align = cur_loc.end % max_align
        cur_loc.padding = 0

        if align:
            cur_loc.padding = max_align - align
            cur_loc.end += cur_loc.padding

        return cur_loc

    def str_decl(self):
        return self.name + '::Interface'

    def get_swap_type(self, settings):
        return SwapType.DontSwap

    def cpp_method(self, context):
        template = '''{type} Base%s() const {{
    int16 off = m(base{name}); if (off == -1) return {{nullptr, lookup}};
    return {{data + off, lookup}};
  }}
'''
        stuff = {
            'name': self.name,
            'type': self.str_decl(),
        }
        context[0].add_unique_line(template.format_map(
            stuff), None, self.name, self.perm)


class ClassMember():
    def __init__(self, name: str, type: NamedType):
        self.name = name
        self.type = type

    def append_offsets(self, offsets: list, cur_offset: int, member_names: list, settings: PermSettings):
        cur_loc = self.type.get_location(cur_offset, settings)
        offsets[member_names.index(self.name)] = cur_loc.begin

        return offsets, cur_loc.end

    def append_swaps(self, swaps: list, member_names: list, settings: PermSettings):
        swaps[member_names.index(
            self.name)] = self.type.get_swap_type(settings)
        return swaps

    def append_names(self, items: list):
        if not self.name in items:
            items.append(self.name)
        return items

    def get_location(self, cur_offset, settings: PermSettings):
        return self.type.get_location(cur_offset, settings)

    def cpp_method(self, context):
        self.func = self.name[0].upper() + self.name[1:]

        if hasattr(self.type, 'cpp_submethod'):
            self.type.cpp_submethod(context, self)
        else:
            stuff = {
                'name': self.name,
                'type': self.type.str_decl(),
            }
            template = None
            if type(self.type) == NamedType:
                template = '{type} %s() const {{ return m({name}) == -1 ? {type}{{}} : *reinterpret_cast<{type}*>(data + m({name})); }}\n'
                set_template = 'void %s({type} value) {{ if (m({name}) >= 0) *reinterpret_cast<{type}*>(data + m({name})) = value; }}\n'
                context[1].add_unique_line(
                    set_template.format_map(stuff), None, self.func, self.perm)
            else:
                template = '{type} %s() const {{ return {{m({name}) == -1 ? nullptr : data + m({name}), lookup}}; }}\n'

            context[0].add_unique_line(
                template.format_map(stuff), None, self.func, self.perm)

    def str_decl(self):
        rval = '%s %s;' % (self.type.str_decl(), self.name)

        return rval


class Inheritable:
    def __init__(self, item: ClassData):
        self.data = item
        self.name = item.name

    def str_decl(self):
        return ''
