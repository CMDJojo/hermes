import { ReactNode } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

import '../styles/Sidebar.css';

interface SidebarProps {
  children: ReactNode;
}

export default function Sidebar({ children }: SidebarProps) {
  return (
    <AnimatePresence>
      <motion.div
        initial={{ width: '0rem' }}
        animate={{ width: '20rem' }}
        exit={{ width: '0rem' }}
        className="Sidebar"
      >
        {children}
      </motion.div>
    </AnimatePresence>
  );
}
