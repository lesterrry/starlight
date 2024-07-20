class MemoryEntry {
  public:
    MemoryEntry(uint16_t address) {
      _address = address;
    }

    template <typename T>
    void write(T value) {
      EEPROM.put(_address, value);
    }

    int32_t read() {
      int32_t retrievedValue;
      EEPROM.get(_address, retrievedValue);
      return retrievedValue;
    }

  private:
    uint16_t _address;
};
