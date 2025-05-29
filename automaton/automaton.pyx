# distutils: language = c++

cimport cython
from libcpp.vector cimport vector
from libcpp.string cimport string

cdef extern from "stdint.h":
    ctypedef unsigned char uint8_t
    ctypedef unsigned long long uint64_t
    ctypedef size_t size_t

cdef extern from "src/automaton.h" namespace "automaton":
    const size_t ROOT

    cdef cppclass Node "automaton::Node":
        uint64_t end
        uint64_t prefix_sum
        double log_end
        double log_prefix_sum
        size_t id
        size_t pre
        size_t fail
        size_t length

    cdef cppclass AutomatonImpl "automaton::Automaton":
        AutomatonImpl() except +
        AutomatonImpl(string dict_path) except +
        AutomatonImpl(vector[string] dict_paths) except +
        void insert(string s, uint64_t freq) except +
        size_t word_count() except +
        Node get_node(size_t node_id) except +
        string get_keyword(size_t node_id) except +
        Node trans_string(string s) except +
        Node trans_byte(uint8_t byte) except +
        vector[Node] get_borders(size_t node_id) except +
        vector[string] get_keywords(string s) except +
        void reset(size_t new_state) except +
        void build(vector[string] dict_paths) except +
        void load_dict(string dict_path) except +

cdef class Automaton:
    cdef AutomatonImpl* autom

    def __cinit__(self, dict_paths = None):
        self.autom = new AutomatonImpl()
        if dict_paths is not None:
            self.build(dict_paths)

    def __dealloc__(self):
        del self.autom

    def insert(self, s, freq = 1):
        cdef string cpp_string = s.encode()
        self.autom.insert(cpp_string, freq)

    def word_count(self) -> int:
        return self.autom.word_count()

    def get_node(self, node_id) -> dict:
        cdef Node node = self.autom.get_node(node_id)
        node_dict = {
            'end': node.end,
            'prefix_sum': node.prefix_sum,
            'log_end': node.log_end,
            'log_prefix_sum': node.log_prefix_sum,
            'id': node.id,
            'pre': node.pre,
            'fail': node.fail,
            'length': node.length
        }
        return node_dict

    def get_keyword(self, node_id) -> str:
        return self.autom.get_keyword(node_id).decode('utf-8')
    
    def trans_string(self, s) -> dict:
        cdef string cpp_string = s.encode()
        cdef Node node = self.autom.trans_string(cpp_string)
        node_dict = {
            'end': node.end,
            'prefix_sum': node.prefix_sum,
            'log_end': node.log_end,
            'log_prefix_sum': node.log_prefix_sum,
            'id': node.id,
            'pre': node.pre,
            'fail': node.fail,
            'length': node.length
        }
        return node_dict

    def trans_byte(self, byte):
        cdef uint8_t cpp_byte = byte
        cdef Node node = self.autom.trans_byte(cpp_byte)
        node_dict = {
            'end': node.end,
            'prefix_sum': node.prefix_sum,
            'log_end': node.log_end,
            'log_prefix_sum': node.log_prefix_sum,
            'id': node.id,
            'pre': node.pre,
            'fail': node.fail,
            'length': node.length
        }
        return node_dict

    def get_borders(self, node_id):
        cdef size_t cpp_node_id = node_id
        cdef vector[Node] borders = self.autom.get_borders(cpp_node_id)
        res = []
        for node in borders:
            node_dict = {
                'end': node.end,
                'prefix_sum': node.prefix_sum,
                'log_end': node.log_end,
                'log_prefix_sum': node.log_prefix_sum,
                'id': node.id,
                'pre': node.pre,
                'fail': node.fail,
                'length': node.length
            }
            res.append(node_dict)
        return res

    def get_keywords(self, s):
        cdef string cpp_string = s.encode()
        cdef vector[string] keywords = self.autom.get_keywords(cpp_string)
        res = []
        for keyword in keywords:
            res.append(keyword.decode('utf-8'))
        return res

    def reset(self, new_state = ROOT):
        self.autom.reset(new_state)

    def build(self, dict_paths=None):
        if isinstance(dict_paths, str):
            dict_paths = [dict_paths]
        cdef vector[string] cpp_dict_paths
        if isinstance(dict_paths, list):
            for dict_path in dict_paths:
                cpp_dict_paths.push_back(dict_path.encode())
        self.autom.build(cpp_dict_paths)

    def load_dict(self, dict_path):
        cdef cpp_dict_path = dict_path.encode()
        self.autom.load_dict(cpp_dict_path)
        
