#![allow(dead_code)] // Allow dead code as not all commands are used in the example

use embassy_stm32::mode::Blocking;
use embassy_stm32::qspi::enums::{
    AddressSize, ChipSelectHighTime, FIFOThresholdLevel, MemorySize, *,
};
use embassy_stm32::qspi::{Config as QspiCfg, Instance, Qspi, TransferConfig};


const MEMORY_PAGE_SIZE: usize = 256;

// Read/Write Array Commands
const CMD_READ: u8 = 0x03;
const CMD_FAST_READ: u8 = 0x0B;
const CMD_TWO_READ: u8 = 0xBB;
const CMD_DUAL_READ: u8 = 0x3B;
const CMD_FOUR_READ: u8 = 0xEB;
const CMD_QUAD_READ: u8 = 0x6B;

const CMD_WRITE_PG: u8 = 0x02;
const CMD_QUAD_WRITE_PG: u8 = 0x38;

const CMD_SECTOR_ERASE: u8 = 0x20;
const CMD_BLOCK_ERASE_32K: u8 = 0x52;
const CMD_BLOCK_ERASE_64K: u8 = 0xD8;
const CMD_CHIP_ERASE: u8 = 0xC7;

// Register/Setting Commands

const CMD_WRITE_ENABLE: u8 = 0x06;
const CMD_WRITE_DISABLE: u8 = 0x04;
const CMD_READ_SR: u8 = 0x05;
const CMD_READ_CR: u8 = 0x15;
const CMD_WRITE_SR_CR: u8 = 0x01;

const CMD_READ_ID: u8 = 0x9F;
const CMD_READ_UUID: u8 = 0x4B;

// ID/Reset Commands
const CMD_ENABLE_RESET: u8 = 0x66;
const CMD_RESET: u8 = 0x99;

const MEMORY_ADDR: u32 = 0x00000000u32;

/// Implementation of access to flash chip.
/// Chip commands are hardcoded as it depends on used chip.
/// This implementation is using chip GD25Q64C from Giga Device
pub struct FlashMemory<I: Instance> {
    qspi: Qspi<'static, I, Blocking>,
}

impl<I: Instance> FlashMemory<I> {
    pub fn new(qspi: Qspi<'static, I, Blocking>) -> Self {
        let mut memory = Self { qspi };
        memory.reset_memory();
        memory.enable_quad();
        memory.enable_hs();
        memory
    }

    fn enable_quad(&mut self) {
        let sr = self.read_sr();
        self.write_sr(sr[0] | 0x40);
        self.wait_write_finish();
    }

    fn enable_hs(&mut self) {
        self.write_cr([0x00, 0x02]);
        self.wait_write_finish();
    }

    fn exec_command(&mut self, cmd: u8) {
        let transaction = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::NONE,
            dwidth: QspiWidth::NONE,
            instruction: cmd,
            address: None,
            dummy: DummyCycles::_0,
        };
        self.qspi.blocking_command(transaction);
    }

    pub fn reset_memory(&mut self) {
        self.enable_write();
        self.exec_command(CMD_ENABLE_RESET);
        self.exec_command(CMD_RESET);
        embassy_time::block_for(embassy_time::Duration::from_micros(20));
    }

    pub fn enable_write(&mut self) {
        self.exec_command(CMD_WRITE_ENABLE);
    }

    pub fn read_id(&mut self) -> [u8; 3] {
        let mut buffer = [0; 3];
        let transaction: TransferConfig = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::NONE,
            dwidth: QspiWidth::SING,
            instruction: CMD_READ_ID,
            address: None,
            dummy: DummyCycles::_0,
        };
        self.qspi.blocking_read(&mut buffer, transaction);
        buffer
    }

    pub fn read_uuid(&mut self) -> [u8; 16] {
        let mut buffer = [0; 16];
        let transaction: TransferConfig = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::SING,
            dwidth: QspiWidth::SING,
            instruction: CMD_READ_UUID,
            address: Some(0),
            dummy: DummyCycles::_8,
        };
        self.qspi.blocking_read(&mut buffer, transaction);
        buffer
    }

    pub fn read_memory(&mut self, addr: u32, buffer: &mut [u8]) {
        let transaction = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::SING,
            dwidth: QspiWidth::QUAD,
            instruction: CMD_QUAD_READ,
            address: Some(addr),
            dummy: DummyCycles::_8,
        };

        self.qspi.blocking_read(buffer, transaction);
    }

    fn wait_write_finish(&mut self) {
        while (self.read_sr()[0] & 0x01) != 0 {}
    }

    fn perform_erase(&mut self, addr: u32, cmd: u8) {
        let transaction = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::SING,
            dwidth: QspiWidth::NONE,
            instruction: cmd,
            address: Some(addr),
            dummy: DummyCycles::_0,
        };
        self.enable_write();
        self.qspi.blocking_command(transaction);
        self.wait_write_finish();
    }

    pub fn erase_sector(&mut self, addr: u32) {
        self.perform_erase(addr, CMD_SECTOR_ERASE);
    }

    pub fn erase_block_32k(&mut self, addr: u32) {
        self.perform_erase(addr, CMD_BLOCK_ERASE_32K);
    }

    pub fn erase_block_64k(&mut self, addr: u32) {
        self.perform_erase(addr, CMD_BLOCK_ERASE_64K);
    }

    pub fn erase_chip(&mut self) {
        self.enable_write();
        self.exec_command(CMD_CHIP_ERASE);
        self.wait_write_finish();
    }

    fn write_page(&mut self, addr: u32, buffer: &[u8], len: usize) {
        assert!(
            (len as u32 + (addr & 0x000000ff)) <= MEMORY_PAGE_SIZE as u32,
            "write_page(): page write length exceeds page boundary (len = {}, addr = {:X}",
            len,
            addr
        );

        let transaction = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::QUAD,
            dwidth: QspiWidth::QUAD,
            instruction: CMD_QUAD_WRITE_PG,
            address: Some(addr),
            dummy: DummyCycles::_0,
        };
        self.enable_write();

        self.qspi.blocking_write(buffer, transaction);

        self.wait_write_finish();
    }

    pub fn write_memory(&mut self, addr: u32, buffer: &[u8]) {
        let mut left = buffer.len();
        let mut place = addr;
        let mut chunk_start = 0;

        while left > 0 {
            let max_chunk_size = MEMORY_PAGE_SIZE - (place & 0x000000ff) as usize;
            let chunk_size = if left >= max_chunk_size {
                max_chunk_size
            } else {
                left
            };
            let chunk = &buffer[chunk_start..(chunk_start + chunk_size)];
            self.write_page(place, chunk, chunk_size);
            place += chunk_size as u32;
            left -= chunk_size;
            chunk_start += chunk_size;
        }
    }

    fn read_register(&mut self, cmd: u8) -> [u8; 2] {
        let mut buffer = [0; 2];
        let transaction: TransferConfig = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::NONE,
            dwidth: QspiWidth::SING,
            instruction: cmd,
            address: None,
            dummy: DummyCycles::_0,
        };
        self.qspi.blocking_read(&mut buffer, transaction);
        buffer
    }

    fn write_register(&mut self, cmd: u8, value: &[u8; 3]) {
        let buffer = *value;
        let transaction: TransferConfig = TransferConfig {
            iwidth: QspiWidth::SING,
            awidth: QspiWidth::NONE,
            dwidth: QspiWidth::SING,
            instruction: cmd,
            address: None,
            dummy: DummyCycles::_0,
        };

        self.qspi.blocking_write(&buffer, transaction);
    }

    pub fn read_sr(&mut self) -> [u8; 2] {
        self.read_register(CMD_READ_SR)
    }

    pub fn read_cr(&mut self) -> [u8; 2] {
        self.read_register(CMD_READ_CR)
    }

    pub fn write_sr(&mut self, value: u8) {
        let cr = self.read_cr();

        let buffer: [u8; 3] = [value, cr[0], cr[1]];

        self.enable_write();
        self.write_register(CMD_WRITE_SR_CR, &buffer);
    }

    pub fn write_cr(&mut self, value: [u8; 2]) {
        let sr = self.read_sr();
        let buffer: [u8; 3] = [sr[0], value[0], value[1]];

        self.enable_write();
        self.write_register(CMD_WRITE_SR_CR, &buffer);
    }
}
