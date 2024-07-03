void EEPROMWrite(unsigned int uiAddress, unsigned char ucData)
{
  while(EECR & (1<<EEWE)) //ждем освобождения флага окончания последней операцией с памятью
  {}
  EEAR = uiAddress; //Устанавливаем адрес
  EEDR = ucData; //Пищем данные в регистр
  EECR |= (1<<EEMWE); //Разрешаем запись
  EECR |= (1<<EEWE); //Пишем байт в память
}

unsigned char EEPROMRead(unsigned int uiAddress)
{
  while(EECR & (1<<EEWE)){} //ждем освобождения флага окончания последней операцией с памятью
  EEAR = uiAddress; //Устанавливаем адрес
  EECR |= (1<<EERE); //Запускаем операцию считывания из памяти в регистр данных
  return EEDR; //Возвращаем результат
}