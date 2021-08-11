using System;	
using Tizen.Peripheral.Spi;

namespace SoundSensor
{
	public class Adc_MCP3008
	{
		private const int BUS = 0;
		private const int CS = 0;

		private const uint SPEED = 3600000;
		private const byte BPW = 8;

		private const byte TX_WORD1 = 0x01;
		private const byte TX_WORD3 = 0x00;
		private readonly byte[] TX_CH = { 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0 };

		private const byte RX_WORD1_MASK = 0x00;
		private const byte RX_WORD2_NULL_BIT_MASK = 0x04;
		private const byte RX_WORD2_MASK = 0x03;
		private const byte RX_WORD3_MASK = 0xFF;
		private const uint UINT10_VALIDATION_MASK = 0x3FF;

		private SpiDevice spiDevice;

		public Adc_MCP3008()
		{
			init();
		}

		public void init()
		{ 
			// open device
			spiDevice = new SpiDevice(BUS, CS)
			{
				Mode = SpiMode.Mode0,
				BitOrder = BitOrder.MSB,
				BitsPerWord = BPW,
				ClockFrequency = SPEED,
			};
		}

		public bool read(int ch_num, ref uint out_value)
		{
			if (spiDevice == null)
				return false;
			if (ch_num < 0 || ch_num > 7)
				return false;


			byte[] tx = { 0x00, 0x00, 0x00 };
			tx[0] = TX_WORD1;
			tx[1] = TX_CH[ch_num];
			tx[2] = TX_WORD3;
			byte[] rx = { 0x00, 0x00, 0x00 };
			spiDevice.TransferSequential(tx, rx);

			byte rx_w1 = (byte)(rx[0] & RX_WORD1_MASK);
			if (rx_w1 != 0x00)
				return false;

			byte rx_w2_nb = (byte)(rx[1] & RX_WORD2_NULL_BIT_MASK);
			if (rx_w2_nb != 0x00)
				return false;

			byte rx_w2 = (byte)(rx[1] & RX_WORD2_MASK);
			byte rx_w3 = (byte)(rx[2] & RX_WORD3_MASK);

			uint result = rx_w2;
			result <<= 8;
			result |= (rx_w3);
			result &= UINT10_VALIDATION_MASK;

			out_value = result;
			return true;
		}

		public void close()
		{
			if (spiDevice == null)
				return;
			spiDevice.Close();
		}
	}
}