class EnhancedList(list):
    """
    Улучшенный list.
    Данный класс является наследником класса list и добавляет к нему несколько новых атрибутов.

    - first -- позволяет получать и задавать значение первого элемента списка.
    - last -- позволяет получать и задавать значение последнего элемента списка.
    - size -- позволяет получать и задавать длину списка:
        - если новая длина больше старой, то дополнить список значениями None;
        - если новая длина меньше старой, то удалять значения из конца списка.
    """

    def int(self, seq=()):
        super.init(seq)

    @property
    def first(self):
        return self[0]

    @first.setter
    def first(self, value):
        self[0] = value;

    @property
    def last(self):
        return self[-1]

    @last.setter
    def last(self, value):
        self[-1] = value

    @property
    def size(self):
        return len(self)

    @size.setter
    def size(self, new_size):
        current_size = len(self)
        if new_size > current_size:
            while(len(self) < new_size):
                self.append(None)
        else:
            while(len(self) > new_size):
                self.pop()
    pass
