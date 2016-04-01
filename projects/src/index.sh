File=symbols
find /usr/include /usr/local/include ../include . -type f | grep -E "\.h$|\.cpp$|\.hpp$|\.c$" > ${File}
cscope -bq -i ${File}
#ctags --c++-kinds=+p --fields=+iaS --extra=+q -L ${File}
ctags -I __THROW -I __attribute_pure__ -I __nonnull -I __attribute__ --file-scope=yes --langmap=c:+.h --languages=c,c++ --links=yes --c-kinds=+p --c++-kinds=+p --fields=+iaS --extra=+q -L ${File}
#rm -rf ${File}

